/* 
 * File:   modem.c
 * Author: robertoalcantara
 *
 * Created on 24 de Março de 2015, 16:28
 */

#include <string.h>
#include <stdio.h>
#include "mcc_generated_files/mcc.h"

#include "globals.h"
#include "modem.h"
#include "expect.h"
#include "flash_io.h"


/* executa o expect em rx_data_available. Se timeout, goto p/ label; se ok, incrementa a variavel de estado D */
#define _expect(A,B,D,C) tmp=expect( rx_data, (unsigned char *) A, B , rx_data_available);  if (EXPECT_TIMEOUT == tmp ) goto C; else if ( EXPECT_FOUND == tmp ) {D++; RX_DATA_ACK; };
#define _expect_keep_buffer(A,B,D,C) tmp=expect( rx_data, (unsigned char *) A, B , rx_data_available);  if (EXPECT_TIMEOUT == tmp ) goto C; else if ( EXPECT_FOUND == tmp ) {D++;};

#define _async_comp(A) strstr( (unsigned char*) rx_data, (const char*) A )
#define _tx(A,B) printf ((unsigned char *) A); B++;
#define _nonblock_wait(A) if ( global_timer.on1seg ) { location_tmp--; if (location_tmp == 0) { A++; } }
#define _nonblock_wait_start(A,B) location_tmp = A; B++;

#define SUCCESS 200 /* Final da maquina de "estado com sucesso*/
#define DELAY_ATCBAND 5 /* delay em s apos um cband - 15 recomendando producao*/

unsigned char state_modem = 0;
unsigned char state_setup = 0;
unsigned char state_location = 0;
unsigned char state_main = 0;
unsigned char state_band = 0;
unsigned char state_enter_gprs = 0;

unsigned char indice_banda = 0;
unsigned char tmp;
unsigned char location_tmp;
unsigned char str_tmp[256];
unsigned char *ptr;

void undervoltage( void );
void modem_async_parser( void );
unsigned char modem_query_erbs ( void );
unsigned char modem_state_machine( void );
unsigned char modem_setup ( void );

#define NUM_BANDS 8
const unsigned char band_modes[NUM_BANDS][16] = { "EGSM_MODE","DCS_MODE","GSM850_MODE","PCS_MODE","EGSM_DCS_MODE","GSM850_PCS_MODE","EGSM_PCS_MODE","ALL_BAND" };




int power_modem( char enable ) {


    if ( enable==1 && PWR_STAT_GetValue()==1) { return; /* Ja ligado */ }
    if ( enable==0 && PWR_STAT_GetValue()==0) { return; /* Ja Desligado */ }

    if ( enable == 0 ) {
        printf("AT+CPOWD=1\r\n");
        return;
    }

    switch (state_modem) {
        case 0:
            MODEM_PWR_SetLow();
            if ( global_timer.on1seg ) state_modem++;
            break;

        case 1:
            MODEM_PWR_SetHigh();
            if ( global_timer.on1seg ) state_modem++;
            break;

        case 2:
            MODEM_PWR_SetLow();
            state_modem++;
            break;
    }

}





void modem_async_parser(void)  {

    if ( _async_comp("UNDER-VOLTAGE WARNING") ) {
        undervoltage();
        RX_DATA_ACK;
        return;
    }
    if ( _async_comp("NORMAL POWER DOWN") ) {
       //enviado pelo modem qdo comandamos o shutdown
        RX_DATA_ACK;
        return;
    }
    if ( _async_comp("+CPIN: READY") ) {
       //enviado pelo modem qdo comandamos o shutdown
        RX_DATA_ACK;
        return;
    }
   if ( _async_comp("SMS") ) {
       //enviado pelo modem qdo chega SMS
        RX_DATA_ACK;
        return;
    }
   if ( _async_comp("ERROR") ) {
       //enviado pelo modem qdo chega SMS
        RX_DATA_ACK;
        return;
    }
    if ( _async_comp("VOLTAGE POWER DOWN") ) {
       //enviado pelo modem qdo comandamos o shutdown
        RX_DATA_ACK;
        return;
    }

}

void undervoltage(void) {
    /*TODO*/
}

unsigned char modem_setup ( void ) {

    switch (state_setup) {

        case 0:
            _tx("AT\r\n", state_setup);
            break;

        case 1:
            _expect("OK", 5, state_setup, setup_error);
            break;

        case 2:
              _tx("ATE1\r\n", state_setup); //echo off
              break;

        case 3:
            _expect("OK", 5, state_setup, setup_error);
            break;

        case 4:
            _tx("AT+CENG=1,1\r\n", state_setup); //eng mode
            break;

        case 5:
            _expect("OK", 2, state_setup, setup_error);
            break;

        case 6:
            _tx("AT+CMEE=2\r\n", state_setup); //eng mode
            break;

        case 7:
            _expect("OK", 2, state_setup, setup_error);
            break;

        case 8:
            return SUCCESS; /* END */

    }
    return 0;

setup_error:
    /* There's nothing to be done. Try again and again. We need a modem. */
    state_setup=0;
    RX_DATA_ACK; //descarta o buffer
    return 0;

}

void fmt_ceng_flash(char* origem) {
    //formata e grava na flash a resposta do CENG
    char ch;

    while ( (ch=*origem++)!= 0  ) {

        switch( ch ) {
                    case '\n' :
                    case 'O' :
                    case 'K' :
                    case 'C' :
                    case 'E' :
                    case 'N' :
                    case 'G' :
                    case ' ' :
                        break;

                    case '+' :
                        ch = '\n';
                        goto def;

                    case ':' :
                        ch = '+';
                        goto def;
def:
                    default:
                        flash_write_char(ch);
                }
    }
    
}

unsigned char modem_query_band( void ) {

    switch( state_band ) {
        case 0:
            sprintf( str_tmp, "AT+CBAND=%s\r\n", band_modes[indice_banda] );
            _tx( str_tmp , state_band);
            break;
        case 1:
            _nonblock_wait_start( DELAY_ATCBAND, state_band );
            break;
        case 2:
            _nonblock_wait(state_band);
            break;
        case 3:
            _tx("AT+CENG?\r\n", state_band);
            break;
 
        case 4:
             _expect_keep_buffer("OK", 5, state_band, band_error);
             /* o buffer eh importante na sequencia, mante-lo intocado*/
             break;
        case 5:
            fmt_ceng_flash( rx_data ); //formata e grava na flash
            RX_DATA_ACK; /*Sinaiza que tratou o buffer*/
            indice_banda++;
            state_band = 0;

            if (indice_banda >= NUM_BANDS)  {
                return SUCCESS;
            }

            break;
    }

    return 0;

band_error:
    state_band = 0;
    RX_DATA_ACK; //descarta o buffer
    return 0;

}

unsigned char modem_query_erbs ( void ) {

    switch (state_location) {

        case 0:
            if (modem_query_band()==SUCCESS) {
              state_location++;
            }
            break;

        case 1:
            flash_write_char('!');
            flash_commit();
            return SUCCESS;
            

    }
    return 0;

}


unsigned char modem_enter_gprs( void ) {

    switch (state_enter_gprs) {

        case 0:
            _tx("AT+CENG=0\r\n", state_enter_gprs);
            break;

        case 1:
            _expect("OK", 5, state_enter_gprs, sa_error);
            break;

        case 2:
            _tx("AT+SAPBR=3,1,\"Contype\",\"GPRS\"\r\n", state_enter_gprs);
            break;

        case 3:
            _expect("OKz", 5, state_enter_gprs, sa_error);
            break;

        case 4:
            _tx("AT+SAPBR=3,1,\"APN\",\"zap.vivo.com.br\"\r\n", state_enter_gprs);
            break;
        
        case 5:
            _expect("OK", 5, state_enter_gprs, gprs_error);
            break;

        case 6:
            _tx("AT+SAPBR=3,1,\"USER\",\"vivo\"\r\n", state_enter_gprs);
            break;

        case 7:
            _expect("OK", 5, state_enter_gprs, gprs_error);
            break;

        case 8:
            _tx("AT+SAPBR=3,1,\"PWD\",\"vivo\"\r\n", state_enter_gprs);
            break;

        case 9:
            _expect("OK", 5, state_enter_gprs, gprs_error);
            break;

        case 10:
            _tx("AT+SAPBR=1,1\r\n", state_enter_gprs);
            break;

        case 11:
            _expect("OK", 5, state_enter_gprs, gprs_error);
            break;
            
        case 99:
            return SUCCESS;

    }

    return 0;
    

sa_error:
 printf("ERROR2\r\n");
    //nao chegou o ok. Precisamos ver o que foi.
    if ( strstr(rx_data, "SIM not inserted") ) {
        /* Nao tem SIM CARD.*/
        SINALIZA_SIM_FAULT;
        state_enter_gprs = 0;
        RX_DATA_ACK; //descarta o buffer
        return 0;
    }


gprs_error:
printf("ERROR\r\n");
    state_enter_gprs = 0;
    RX_DATA_ACK; //descarta o buffer
    return 0;
}



/* Maquina de estados do modem
 * Verifica se ele esta no estado de energia que deveria.
 *
 */
unsigned char modem_handler(void) {

    power_modem( modem_power_status ); //maquina de estado de configuracao do modem

    if ( 1 == modem_power_status ) {
        /* Sem o modem estar ligado nao faz sentido executar a maquina...*/
        switch(state_main) {

            case 0:
                /* Zerar  a maquina de configuracao do modem */
                state_setup = 0;
                state_main++;
                break;

            case 1:
                /* Esperar a maquina de config do modem encerrar */
                if (modem_setup() == SUCCESS) {
                    state_main++;
                    state_location = 0; /* Zera a maq de estado de query de redes */
                    state_band = 0; /* Zera a maq de estado da sequencia das bandas */

                    state_main = 2; /** DEBUG !!!! **/
                }
                break;

            case 2:
                if (modem_query_erbs() == SUCCESS) {
                    state_enter_gprs = 0;
                    state_main++;
                }
                break;

            case 3:
                if (modem_enter_gprs() == SUCCESS) {
                    state_main++;
                }
                break;
        }
        //DEBUGif (global_timer.on1seg) printf("E: %d G: %d\n\r", state_main, state_gprs);

    }

    
    return (1);
}






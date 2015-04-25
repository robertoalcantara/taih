/* 
 * File:   modem.c
 * Author: robertoalcantara
 *
 * Created on 24 de Março de 2015, 16:28
 */

#include <string.h>
#include <stdio.h>
#include "globals.h"
#include "modem.h"
#include "expect.h"

/* executa o expect em rx_data_available. Se timeout, goto p/ label; se ok, incrementa a variavel de estado D */
#define _expect(A,B,D,C) tmp=expect( rx_data, (unsigned char *) A, B , rx_data_available);  if (EXPECT_TIMEOUT == tmp ) goto C; else if ( EXPECT_FOUND == tmp ) {D++; RX_DATA_ACK; };
#define _expect_keep_buffer(A,B,D,C) tmp=expect( rx_data, (unsigned char *) A, B , rx_data_available);  if (EXPECT_TIMEOUT == tmp ) goto C; else if ( EXPECT_FOUND == tmp ) {D++;};

#define _async_comp(A) strstr( (unsigned char*) rx_data, (const char*) A )
#define _tx(A,B) printf ((unsigned char *) A); B++;
#define _nonblock_wait(A) if ( global_timer.on1seg ) { location_tmp--; if (location_tmp == 0) { A++; } }
#define _nonblock_wait_start(A,B) location_tmp = A; B++;

#define SUCCESS 200 /* Final da maquina de estado com sucesso*/
#define DELAY_ATCBAND 20 /* delay em s apos um cband - 10 recomendando producao*/

unsigned char state_setup = 0;
unsigned char state_location = 0;
unsigned char state_main = 0;
unsigned char state_band = 0;


unsigned char indice_banda = 0;
unsigned char tmp;
unsigned char location_tmp;
unsigned char str_tmp[50];
unsigned char buffer_str[900];
unsigned char *ptr;

void undervoltage( void );
void modem_async_parser( void );
unsigned char modem_query_erbs ( void );
unsigned char modem_state_machine( void );
unsigned char modem_setup ( void );

#define NUM_BANDS 8
const unsigned char band_modes[NUM_BANDS][16] = { "EGSM_MODE","DCS_MODE","GSM850_MODE","PCS_MODE","EGSM_DCS_MODE","GSM850_PCS_MODE","EGSM_PCS_MODE","ALL_BAND" };





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
              _tx("ATE0\r\n", state_setup); //echo off
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
            return SUCCESS; /* END */

    }
    return 0;

setup_error:
    /* There's nothing to be done. Try again and again. We need a modem. */
    state_setup=0;
    RX_DATA_ACK; //descarta o buffer
    return 0;

}

void strcat_ceng(char* destino, char* origem) {
    //apenas concatena em destino a origem sem determinados caracteres (formato ceng)
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
                        *destino = ch;
                        *destino++;
                        break;

                    case ':' :
                        ch = '+';
                        *destino = ch;
                        *destino++;
                        break;

                    default:
                        *destino = ch;
                        *destino++;
                }
    }
    *destino = 0; //termina a string
    
}

unsigned char modem_query_band( void ) {

    unsigned char idx;

    if (indice_banda >= NUM_BANDS)  {
        return SUCCESS;
    }

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
             idx = 0;
             /* o buffer eh importante na sequencia, mante-lo intocado*/
             break;
        case 5:
            strcat_ceng( buffer_str, rx_data );
            //strcat( buffer_str, rx_data ); //Funciona ok, mas leva muito lixo.

            RX_DATA_ACK; /*Sinaiza que tratou o buffer*/
            /* chegou os dados da banda
             * at+ceng?
                +CENG: 1,0

                +CENG: 0,"0799,61,00,724,11,18,61ab,08,00,4fa1,255"
                +CENG: 1,"0678,43,60,724,11,4fa1"
                +CENG: 2,"0664,37,33,724,11,4fa1"
                +CENG: 3,"0686,27,13,724,11,4fa1"
                +CENG: 4,"0802,26,52,724,11,4fa1"
                +CENG: 5,"0796,25,16,724,11,4fa1"
                +CENG: 6,"0806,48,53,000,00,0"

                OK
              */
            indice_banda++;
            state_band = 0;
            break;
    }

    return 0;

band_error:
    state_band = 0;
    strcpy(buffer_str,"");
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
            return SUCCESS;

    }
    return 0;

}

/* Maquina de estados global*/
unsigned char modem_state_machine(void) {


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
                strcpy(buffer_str,"");
            }
            break;

        case 2:
            if (modem_query_erbs() == SUCCESS) {
                state_main++;
                /*ToDo*/
            }
            break;
    }
    
    return 0;
}






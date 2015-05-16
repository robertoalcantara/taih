/* 
 * File:   modem.c
 * Author: robertoalcantara
 *
 * Created on 24 de Março de 2015, 16:28
 */

#include <string.h>
#include <stdio.h>
#include <xc.h>

#include "mcc_generated_files/mcc.h"

#include "globals.h"
#include "modem.h"
#include "expect.h"
#include "flash_io.h"


/* executa o expect em rx_data_available. Se timeout, goto p/ label; se ok, incrementa a variavel de estado D */
#define _expect(A,B,D,C) exp=expect( rx_data, (char *) A, B , rx_data_available);  if (EXPECT_TIMEOUT == exp ) goto C; else if ( EXPECT_FOUND == exp ) {D++;RX_DATA_ACK};
#define _expect_keep_buffer(A,B,D,C) exp=expect( rx_data, ( char *) A, B , rx_data_available);  if (EXPECT_TIMEOUT == exp ) goto C; else if ( EXPECT_FOUND == exp ) {D++;};

#define _async_comp(A) strstr( rx_data, A )

#define _tx(A,B) printf ((char *) A); B++; printD(A); RX_DATA_ACK;
#define _nonblock_wait(A) if ( global_timer.on1seg ) { location_tmp--; if (location_tmp == 0) { A++; } }
#define _nonblock_wait_start(A,B) location_tmp = A; B++;

#define DELAY_ATCBAND 20 /* delay em s apos um cband - 15 recomendando producao*/

unsigned char state_modem = 0;
unsigned char state_setup = 0;
unsigned char state_location = 0;
unsigned char state_main = 99; // teste comecando fora
unsigned char state_band = 0;
unsigned char state_enter_gprs = 0;
unsigned char state_tx_http = 0;


unsigned char indice_banda = 0;
unsigned char exp;
unsigned char location_tmp;
unsigned char str_tmp[256];
unsigned char *ptr;

unsigned long http_pack_len = 0;

void undervoltage( void );
void modem_async_parser( void );
unsigned char modem_query_erbs ( void );
unsigned char modem_state_machine( void );
unsigned char modem_setup ( void );

#define NUM_BANDS 8
const unsigned char band_modes[NUM_BANDS][16] = { "EGSM_MODE","DCS_MODE","GSM850_MODE","PCS_MODE","EGSM_DCS_MODE","GSM850_PCS_MODE","EGSM_PCS_MODE","ALL_BAND" };

void delay_10ms(unsigned long vl ) {

    for (int i=0; i<=vl; i++)
    __delay_ms(10);
}


int power_modem( char enable ) {

    while ( PWR_STAT_GetValue() != enable ) {
        MODEM_PWR_SetLow();
        LED_D7_SetLow();
        delay_10ms(100);
        MODEM_PWR_SetHigh();
        LED_D7_SetHigh();
        delay_10ms(150);
        MODEM_PWR_SetLow();
        LED_D7_SetLow();
        delay_10ms(300); 
    }

}





void modem_async_parser(void)  {
    
    if ( _async_comp("UNDER-VOLTAGE WARNING") ) {
        printD("assync parser: Undervoltage!");
        undervoltage();
        RX_DATA_ACK;
        return;
    }
    if ( _async_comp("NORMAL POWER DOWN") ) {
       //enviado pelo modem qdo comandamos o shutdown
        printD("assync parser: Normal power Down");
        RX_DATA_ACK;
        return;
    }

    if ( _async_comp("SIM not inserted") ) {
       //enviado pelo modem qdo comandamos o shutdown
        printD("assync parser: SIM nao Inserido");
        RX_DATA_ACK;
        return;
    }

    if ( _async_comp("+CPIN: READY") ) {
       //enviado pelo modem qdo comandamos o shutdown
        printD("assync parser: CPIN READY!");
        RX_DATA_ACK;
        return;
    }
   if ( _async_comp("SMS") ) {
       //enviado pelo modem qdo chega SMS
       printD("assync parser: SMS!");
        RX_DATA_ACK;
        return;
    }
   if ( _async_comp("ERROR") ) {
       printD("assync parser: ERROR (SMS?)");
       printD(rx_data);

       //enviado pelo modem qdo chega SMS
        RX_DATA_ACK;
        return;
    }
    if ( _async_comp("VOLTAGE POWER DOWN") ) {
            printD("assync parser: desligando modem");

       //enviado pelo modem qdo comandamos o shutdown
        RX_DATA_ACK;
        return;
    }

    if ( _async_comp("+CME ERROR: operation not allowed") ) {
       //Erro qdo nao consegue entrar na rede gprs
        printD("\r\nErro ao gprs  async  CME Error\r\n");
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
            _tx("AT+CMEE=2\r\n", state_setup); //eng mode
            break;

        case 7:
            _expect("OK", 2, state_setup, setup_error);
            break;

        case 8:
           state_setup++;
           printD("modem_setup: SUCCESS");
           return SUCCESS; /* END */

    }
    return 0;

setup_error:
    printD("modem_setup: setup_ERROR");

    /* There's nothing to be done. Try again and again. We need a modem. */
    state_setup=0;
    EXPECT_ERROR;
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
                        http_pack_len++;
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
            //gravando:
            printD( "grava flash:" );
            printD( rx_data );
            
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
    EXPECT_ERROR;
    RX_DATA_ACK; //descarta o buffer
    return 0;

}

unsigned char modem_query_erbs ( void ) {

    switch (state_location) {

        case 0:
            state_band = 0;
            state_location++;

        case 1:
            if (modem_query_band()==SUCCESS) {
              state_location++;
            }
            break;

        case 2:
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
           _expect("OK", 5, state_enter_gprs, sa_error);
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
            _expect("OK", 15, state_enter_gprs, gprs_error);
            break;
            
        case 12:
            state_enter_gprs++;
            printD("modem_enter_gprs: SUCCESS");
            return SUCCESS;

    }

    return 0;
    

sa_error:
    printD("modem_enter_gprs: SA_ERROR");
    //nao chegou o ok. Precisamos ver o que foi.
    if ( strstr(rx_data, "SIM not inserted") ) {
        // Nao tem SIM CARD.
        SINALIZA_SIM_FAULT;
        printD("modem_enter_gprs: SIM CARD ERROR");

    }

gprs_error:
   printD("modem_enter_gprs: GPRS_ERROR");

    EXPECT_ERROR;
    printf("\r\nERR2 modem_enter_gprs\r\n");
    state_enter_gprs = 0;
    RX_DATA_ACK; //descarta o buffer
    return 0;
}



unsigned char modem_tx_http( void ) {
    unsigned long count = 0;
    uint32_t flashAdd;
    unsigned char ch;
    unsigned int http_tx_len;

    switch (state_tx_http) {

        case 0:
            flashAdd = BASE_ADDR; //endereco inicial na flash para ler
            _tx("AT+HTTPINIT\r\n", state_tx_http);
            break;

        case 1:
            _expect("OK", 5, state_tx_http, http_error);
            break;

        case 2:
            _tx("AT+HTTPPARA=\"CID\",1\r\n", state_tx_http);
            break;

        case 3:
            _expect("OK", 5, state_tx_http, http_error);
            break;

        case 4:
            _tx("AT+HTTPPARA=\"URL\",\"http://50.16.199.44/api/device\"\r\n", state_tx_http);
            //          _tx("AT+HTTPPARA=\"URL\",\"http://50.16.199.44:2000\"\r\n", state_tx_http);
            break;

        case 5:
            _expect("OK", 5, state_tx_http, http_error);
            break;

        case 6:
            _tx("AT+HTTPPARA=\"CONTENT\",\"application/x-www-form-urlencoded\"\r\n", state_tx_http);
            break;

        case 7:
            _expect("OK", 5, state_tx_http, http_error);
            break;

        case 8: //mesmo que o _tx mas com parametro no printf...  muito cuidado com os tamanhos ;)  o for eh http_len_pack
            sprintf( str_tmp, "%d", vbat);
            http_tx_len = http_pack_len + 10 + 2 + strlen(str_tmp);  //id=1&data=  +! +@
            printf("AT+HTTPDATA=%d,20000\r\n", http_tx_len);
            state_tx_http++;
            RX_DATA_ACK;
            break;

        case 9:
            _expect("DOWNLOAD", 5, state_tx_http, http_error);
            break;

        case 10:
            _tx("id=1&data=", state_tx_http);
            for (count=0; count<http_pack_len; count++) {
                ch = (unsigned char)FLASH_ReadByte(flashAdd);
                flashAdd++;
                EUSART1_Write(ch); //transmitindo para a principal
                EUSART2_Write(ch); //transmitindo para o debug
            }
            EUSART1_Write('!');
            printf("%s", str_tmp);
            EUSART1_Write('@');
            break;

        case 11:
            _expect("OK", 10, state_tx_http, http_error);
            break;

        case 12:
            _tx("AT+HTTPACTION=1\r\n", state_tx_http);
            break;

        case 13:
            _expect("+HTTPACTION", 10, state_tx_http, http_error);
            break;

        case 14:
            _tx("AT+HTTPTERM\r\n", state_tx_http); //detach da rede
            break;

        case 15:
            _expect("OK", 5, state_tx_http, http_error);
            break;


        case 16:
            _tx("AT+CGATT=0\r\n", state_tx_http); //detach da rede
            break;
        
        case 17:
            _expect("OK", 5, state_tx_http, http_error);
            break;

        case 18:
            state_tx_http++;
            
            printf("AT+CPOWD=1\r\n");

            printD("\r\nFIM SUCC\r\n");
            return SUCCESS;

    }

    return 1;

http_error:
    printD("n\r ERR http_error \n\r");
    EXPECT_ERROR;
    RX_DATA_ACK;
    state_tx_http = 0;
    return 0;


}


/* Maquina de estados do modem
 * Verifica se ele esta no estado de energia que deveria.
 *
 */
unsigned char modem_handler(void) {
    static unsigned long modem_global_timeout = 0;

    if (PWR_STAT_GetValue() != modem_power_status) { return 0; } //FAULT
    
    if ( modem_power_status ) {

          /* Modem Ligado... */

        if ( global_timer.on1seg ) {
            //tempo maximo de execucao da maquina completa de 500s.
            modem_global_timeout++;
            if (modem_global_timeout >= TIMEOUT_STATE_MODEM) {
                modem_global_timeout = 0;
                state_main = 0;
                printD("\r\nGLB TIMEOUT\r\n");

            }
        }

        switch(state_main) {

            case 0:
                /* Zerar  a maquina de configuracao do modem */
                state_setup = 0;
                state_main++;
                http_pack_len = 0; //tamanho do payload
                printD("indo p/ state_main: 1");
                break;

            case 1:
                /* Esperar a maquina de config do modem encerrar */
                if (modem_setup() == SUCCESS) {
                    state_main++;
                    state_location = 0; /* Zera a maq de estado de query de redes */
                    state_band = 0; /* Zera a maq de estado da sequencia das bandas */
                    state_enter_gprs = 0;
                    RESET_FLASH; //inicia o ponteiro da flash. just in case (commit ao final tb o fara).
                    printD("indo p/ state_main: 2");

                }
                break;

            case 2:
                if (modem_query_erbs() == SUCCESS) {
                    state_enter_gprs = 0;
                    state_main++;
                    printD("indo p/ state_main: 3");
                }
                break;

            case 3:
                if (modem_enter_gprs() == SUCCESS) {
                    state_main++;
                    state_tx_http = 0;
                    printD("indo p/ state_main: 4");
                }
                break;

            case 4:
                if (modem_tx_http()==SUCCESS) {
                    state_main++;
                    printD("indo p/ state_main: 5");
                }
                break;
                
            case 5:
                printD("state_main: 5");
                printD("MAIN SCUCESS");
                state_main++;
                return SUCCESS;
        }

    }
    
    
    return 1;
}






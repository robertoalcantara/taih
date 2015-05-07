/* 
 * File:   main.c
 * Author: robertoalcantara
 *
 * Created on 24 de Abril de 2015, 16:00
 *
 * Offset 0x4000
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "mcc_generated_files/mcc.h"
#include "mcc_generated_files/memory.h"
#include "mcc_generated_files/adc.h"

#include "globals.h"
#include "modem.h"

volatile T_GLOBAL_TIMER global_timer;


unsigned char rx_data_available = 0;
unsigned char rx_data[RX_BUFFER_SIZE];
unsigned int rx_data_index = 0;

unsigned char battery_level = 0;

unsigned char modem_power_status = 0;

unsigned char sinalizacao_status = 0;



void printD(const char* str) {
    while (*str) {
        EUSART2_Write(*str);
        str++;
    }
    EUSART2_Write('\r');
    EUSART2_Write('\n');
}


/*
 *
 */
void setup (void) {

    SYSTEM_Initialize();
    
    INTERRUPT_GlobalInterruptEnable();
    INTERRUPT_PeripheralInterruptEnable();

    channel_AN4_SetAnalogMode();
    VBAT_CONTROL_SetLow();
    LED_D6_SetHigh(); // Ligando

    global_timer.aux_100ms = 0;
    global_timer.aux_10ms = 0;
    global_timer.aux_1s = 0;

    printD("Serial DEBUG");

}


void check_vbat(void){
    unsigned long vbat;
    char tmp[15];


    ADC_Initialize();
    vbat = ADC_GetConversion(channel_AN4);

    //sprintf(tmp,"B:%lu", vbat);
    //printD(tmp);
}


void handler_sinalizacao(void) {
    static unsigned char cnt_live = 0;
    static unsigned char cnt_ack = 0;

    if ( global_timer.on1seg ) cnt_live++;

    if ( global_timer.on100ms ) cnt_ack++;


    switch (sinalizacao_status & 0x0F) {
        case SINALIZACAO_NORMAL:
            if ( global_timer.on1seg ) { LED_D6_Toggle(); LED_D7_SetLow(); }
            /*if ( 2 == cnt_live ) {
                cnt_live = 0;
                LED_D6_SetHigh();
            } else {
                if (global_timer.on100ms) {
                    LED_D6_SetLow();
                }
            }*/
            break;

        case SINALIZACAO_MODEM_FAULT:
            if ( global_timer.on100ms) {
                LED_D6_Toggle();
                LED_D7_LAT = !LED_D6_LAT;
            }
            return; /* Afeta o D7, nem olha as sinalizacoes temporarias */

        case SINALIZACAO_SIM_FAULT:
            if ( global_timer.on100ms) {
                LED_D6_Toggle();
                LED_D7_LAT = LED_D6_LAT;
            }
            return; /* Afeta o D7 */
    }

    //essa sinalizaco eh do tipo temporaria
    switch (sinalizacao_status & 0xF0) {
        case SINALIZACAO_MSG_ACK:
            LED_D7_SetHigh();
            if ( global_timer.on10ms) {
                sinalizacao_status = (0x0F & sinalizacao_status);
                LED_D7_SetLow();
            }
            break;
            
            
    }
}


void serial_buffer_copy(void){
    char aux;

    if (rx_data_available) { /* Buffer overrun! */ }

    while ( eusart1RxCount > 0  ) {
        aux = EUSART1_Read();

        if (aux == '\r') continue;
        rx_data[ rx_data_index ] = aux;
        rx_data_index++;
        if (rx_data_index >= RX_BUFFER_SIZE) {
            //assert! vai dar buffer overflow.
            printD("\r\nASSERT Buffer Overflow serial_buffer_copy\r\n");
            rx_data_index = 0;
            rx_data[0] = 0;
            return;
        }

        rx_data[ rx_data_index ] = 0; //garantindo sempre...

        if ( aux == '\n' ) {
            SINALIZA_MSG_ACK;
            rx_data_available = 1;
            break;
        }


    }

}


/*
 * 
 */
int main() {
    long x, y;
    char cnt = 0;
    char ret = 0;
    unsigned long cnt_tempo_transmissao = 590; //comecar mandando pra testar
    unsigned char cnt_modem_fault = 0;

    setup ();

    MODEM_DISABLE;

    while (1) {
        SINALIZA_NORMAL;

        if ( global_timer.on1seg ) {
            cnt_tempo_transmissao++; 
            power_modem( modem_power_status );  //maquina de estado de configuracao do modem
            check_vbat();
        }

    
        // Verifica se existe dado na serial para processar
        serial_buffer_copy();

        
        modem_async_parser(); //Ja analiza as mensagens assincronas  PROBLEMA AQUI?ANALIZAR COM CUIDADO

        if ( cnt_modem_fault >= 15 ) {
           cnt_modem_fault = 0;
           if (modem_power_status == 1) {
               MODEM_ENABLE;
           } else {
               MODEM_DISABLE;
           }
        }

        ret = modem_handler();
 
        // Maquina do Modem rodando
        if ( cnt_tempo_transmissao == 600 ) { //em segundos
            if ( global_timer.on1seg ) {
                MODEM_ENABLE;
                state_main = 0; //iniciando pra valer a maquina de estado do modem, comecou em 99
                cnt_modem_fault = 0;
                printD("main - cnt_tempo_transmissao START");
            }
           
        }

        if (0 == ret ) {
            // Modem nao esta como deveria
            if ( global_timer.on1seg ) {
                printD("main - Modem Fault"); 
                cnt_modem_fault++;
            }
            SINALIZA_MODEM_FAULT;
            goto error;

        } else {
            //maquina do modem nao retornou erro
            cnt_modem_fault = 0;
            if ( SUCCESS == ret ) {
                //tudo certo
                MODEM_DISABLE;
                cnt_tempo_transmissao = 0;
                if ( global_timer.on1seg ) { printD("main - SUCESSO - desligando modem"); }
            }
        }



error:

        handler_sinalizacao();

        /* flags de tempo */
        global_timer.on1seg  = 0;
        global_timer.on100ms = 0;
        global_timer.on10ms  = 0;
        global_timer.on1ms  = 0;

        while (global_timer.on1ms == 0) { /* Fazer nada */  }

    }

    return (EXIT_SUCCESS);
}





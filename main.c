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
#include <xc.h>

#include "mcc_generated_files/mcc.h"
#include "mcc_generated_files/memory.h"
#include "mcc_generated_files/adc.h"

#include "globals.h"
#include "modem.h"


#ifdef DEBUG
    #define TEMPO_TRANSMISSAO 600  
#else
    #define TEMPO_TRANSMISSAO 600
#endif

volatile T_GLOBAL_TIMER global_timer;


unsigned char rx_data_available = 0;
unsigned char rx_data[RX_BUFFER_SIZE];
unsigned int rx_data_index = 0;

unsigned char battery_level = 0;

unsigned char modem_power_status = 0;

unsigned char sinalizacao_status = 0;
unsigned long vbat;

unsigned char low_speed = 0;



void printD(const char* str) {
#ifdef DEBUG
    while (*str) {
        EUSART2_Write(*str);
        str++;
    }
    EUSART2_Write('\r');
    EUSART2_Write('\n');
#endif
}


/*
 *
 */
void setup (void) {

    SYSTEM_Initialize(); //serial e AD NAO inicializados para economizar energia.
    
    INTERRUPT_GlobalInterruptEnable();
    INTERRUPT_PeripheralInterruptEnable();

    channel_AN4_SetAnalogMode();

    global_timer.aux_100ms = 0;
    global_timer.aux_10ms = 0;

    #ifdef DEBUG
            EUSART2_Initialize();
    #endif

}


unsigned char check_vbat(void){
    
/* Bateria ja muito baixa. Modem morre com <747 */
    
#ifdef DEBUG
    char tmp[15];
#endif
    ADC_Initialize();
    VBAT_CONTROL_SetLow();
    vbat = ADC_GetConversion(channel_AN4);
    VBAT_CONTROL_SetHigh();

#ifdef DEBUG
    sprintf(tmp,"B:%lu", vbat);
    printD(tmp);
#endif
    

    if (vbat <= LOW_BATTERY_LIMIT) {
        return LOW_BATTERY;
    }

    return 1;
}


void handler_sinalizacao(void) {
    static unsigned char cnt_live = 0;
    static unsigned char cnt_ack = 0;


    if ( global_timer.on1seg ) cnt_live++;

    if ( global_timer.on100ms ) cnt_ack++;


    switch (sinalizacao_status & 0x0F) {
        case SINALIZACAO_NORMAL:
            if ( 3 == cnt_live ) {
                cnt_live = 0;
                LED_D6_SetHigh();
            } else {
                LED_D6_SetLow();
               // LED_D7_SetLow();
            }
            break;

        case SINALIZACAO_MODEM_FAULT:
            if ( global_timer.on100ms) {
                LED_D6_Toggle();
            }
            break;

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
//            LED_D7_SetHigh();
            if ( global_timer.on10ms) {
                sinalizacao_status = (0x0F & sinalizacao_status);
//                LED_D7_SetLow();
            }
            break;            
    }
}


void serial_buffer_copy(void){
    char aux;

    while ( eusart1RxCount > 0  ) {
        aux = EUSART1_Read();

        if (aux == '\r') continue;
        rx_data[ rx_data_index ] = aux;
        rx_data_index++;
        if (rx_data_index >= RX_BUFFER_SIZE) {
            //assert! vai dar buffer overflow.
            printD("\r\nASSERT Buffer Overflow serial_buffer_copy\r\n");
            RX_DATA_ACK;
            return;
        }

        rx_data[ rx_data_index ] = 0; //garantindo sempre...

        if ( aux == '\n' ) {

            //detectar enter vazio (linha em branco)
            if (rx_data_index == 1) { rx_data_index = 0; continue; }

            if (rx_data_available) { /* Buffer overrun OU conjunto de mensagens com varios \n! */ }
            //SINALIZA_MSG_ACK;
            rx_data_available = 1;
            break;
        }
    }

}


/*
 * 
 */
int main() {
    char ret = 0;
    unsigned long cnt_tempo_transmissao = 0; //comecar mandando pra testar
    unsigned char cnt_modem_fault = 0;
    static unsigned char flag_low_bat = 0;

    setup ();


    MODEM_DISABLE;
    TMR0ON = 0; //timer de 1ms fica desligado

    OSCCON = 0x02; //31.25k  modo lento
    low_speed = 1;

    while ( low_speed ) {
        //low speed on!

        if ( (0==modem_power_status) && 1==PWR_STAT_GetValue() ) {
            MODEM_DISABLE;
        }

        cnt_tempo_transmissao = cnt_tempo_transmissao + 10;
        if ( cnt_tempo_transmissao == TEMPO_TRANSMISSAO ) { 
            low_speed = 0;
        }

        LED_D6_SetHigh();
        __delay_ms(1); //64 vezes mais lento...
        LED_D6_SetLow();

        ClrWdt();
        IDLEN = 1; //sleep entra em modo idle
        Sleep(); // So sai na interrupcao do tmr0 ou tmr1  (1seg ou 1ms)
    }

    OSCCON = 0x42; //high speed!
    TMR1ON = 0; //nao precisa mais dele
    flag_low_bat = 0;
    TMR0ON = 1; //ligando todas as contagens e nao mais so a de 1s

    ADC_Initialize();
    EUSART1_Initialize();

    MODEM_ENABLE;
    state_main = 0; //iniciando pra valer a maquina de estado do modem, comecou em 99
    cnt_modem_fault = 0;

    printD("main - cnt_tempo_transmissao START");

    check_vbat();

    while (1) {
        
        SINALIZA_NORMAL;
        if (global_timer.on100ms){
   LED_D6_Toggle();
        }

  
        ret = modem_handler();

        serial_buffer_copy();
        //modem_async_parser(); //TEM BUXU!!!Ja analiza as mensagens assincronas  PROBLEMA AQUI?ANALIZAR COM CUIDADO
        
    
        if (0 == ret ) {
            // Modem nao esta como deveria
            if ( global_timer.on1seg ) {
                printD("main - Modem Fault"); 
                cnt_modem_fault++;
            }
            SINALIZA_MODEM_FAULT;

        } else {
            //maquina do modem nao retornou erro
            cnt_modem_fault = 0;
            
            if ( SUCCESS == ret ) {
                Reset();  //Resetando tudo e voltando para o modo baixo consumo.
            }
        }


        if ( cnt_modem_fault >= 3 ) {
           printD("main - modem fault>=3)");
           state_main = 0; //iniciando novamente a maquina do modem

           if (modem_power_status == 1) {
               MODEM_ENABLE;

           } else {
               MODEM_DISABLE;
           }

        }

        

       
     if ( 0 == flag_low_bat) {
        //se a bateria estiver baixa nao sinalizar no handler. Sinalizacao propria no vbat
       // handler_sinalizacao();
     }
        /* flags de tempo */

        global_timer.on1seg  = 0;
        global_timer.on100ms = 0;
        global_timer.on10ms  = 0;
        global_timer.on1ms  = 0;

        ClrWdt();
        IDLEN = 1; //sleep entra em modo idle
        Sleep(); // So sai na interrupcao do tmr0 ou tmr1  (1seg ou 1ms)

    }

    return (EXIT_SUCCESS);
}





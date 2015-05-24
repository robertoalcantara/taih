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

#define DEBUG 1

#ifdef DEBUG
    #define TEMPO_TRANSMISSAO 600   //transmissao a cada 30 minutos.
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
            if ( 7 == cnt_live ) {
                cnt_live = 0;
                LED_D6_SetHigh();
            } else {
                LED_D6_SetLow();
                LED_D7_SetLow();
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

            //detectar enter vazio (linha em branco)
            if (rx_data_index == 1) { rx_data_index = 0; continue; }

            if (rx_data_available) { /* Buffer overrun OU conjunto de mensagens com varios \n! */ }

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
    char ret = 0;
    unsigned long cnt_tempo_transmissao = 0; //comecar mandando pra testar
    unsigned char cnt_modem_fault = 0;
    static unsigned char flag_low_bat = 0;

    setup ();

    TMR0ON = 0; //timer de 1ms fica desligado

    MODEM_DISABLE;


    while (1) {
        
       SINALIZA_NORMAL;

      
       if ( global_timer.on1seg ) {
            cnt_tempo_transmissao++;
            //Workarround para o modem que fica ligando:
            if ( (0==modem_power_status) && 1==PWR_STAT_GetValue() ) {
                MODEM_DISABLE;
            }
       }

        // Maquina do Modem rodando
        if ( cnt_tempo_transmissao == TEMPO_TRANSMISSAO ) { //em segundos
#ifdef DEBUG
            EUSART2_Initialize();
#endif
            ADC_Initialize();

            if ( check_vbat() == LOW_BATTERY) {
                /* Nao da mais pra ligar o modem */
                flag_low_bat = 1;
                printD("main - LOW BATTERY");
                cnt_tempo_transmissao = TEMPO_TRANSMISSAO - 10; //forcando passar aqui e checar a bateria novamente em 10s
                LED_D6_SetHigh();
                LED_D7_SetHigh();
                __delay_ms(10);
                LED_D6_SetLow();
                LED_D7_SetLow();

                goto battery_error;
            }
            if ( 1 == flag_low_bat ) {
                //ja esta conectado ao carregador, mas estava em bateria baixa.
                //histerese do carregador!
                if (vbat <= LOW_BATTERY_HISTERESYS_LIMIT) {
                    LED_D6_SetHigh();
                    LED_D7_SetHigh();
                    __delay_ms(10);
                    LED_D6_SetLow();
                    LED_D7_SetLow();
                    goto battery_error;
                }
            }

            flag_low_bat = 0;
            TMR0ON = 1; //ligando todas as contagens e nao mais so a de 1s
            EUSART1_Initialize();

            MODEM_ENABLE;
            state_main = 0; //iniciando pra valer a maquina de estado do modem, comecou em 99
            cnt_modem_fault = 0;
            cnt_tempo_transmissao++;
            printD("main - cnt_tempo_transmissao START");
        }
      
        ret = modem_handler();

        // Verifica se existe dado na serial para processar
        if (modem_power_status) {
            //so verifica se o modem estiver ligado.
            serial_buffer_copy();
            modem_async_parser(); //Ja analiza as mensagens assincronas  PROBLEMA AQUI?ANALIZAR COM CUIDADO
        }

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
                //TUDO Certo - retornou o fim da maquina.
                cnt_tempo_transmissao = 0;
                Reset();  //Resetando tudo e voltando para o modo baixo consumo.
            }
        }


        if ( cnt_modem_fault >= 30 ) {
           printD("main - modem fault>30)");
           state_main = 0; //iniciando pra valer a maquina de estado do modem, comecou em 99
           cnt_modem_fault = 0;
           if (modem_power_status == 1) {
               MODEM_ENABLE;

           } else {
               MODEM_DISABLE;
           }
        }

        

        
battery_error:  

     if ( 0 == flag_low_bat) {
        //se a bateria estiver baixa nao sinalizar no handler. Sinalizacao propria no vbat
        handler_sinalizacao();
    } else {
       LED_D6_SetLow();
       LED_D7_SetLow();
    }
 
        /* flags de tempo */

        global_timer.on1seg  = 0;
        global_timer.on100ms = 0;
        global_timer.on10ms  = 0;
        global_timer.on1ms  = 0;


        ClrWdt();
        IDLEN = 1; //sleep entra em modo idle
        SCS1 = 1;
        SCS0 = 0;
        Sleep(); // So sai na interrupcao do tmr0 ou tmr1  (1seg ou 1ms)

    }

    return (EXIT_SUCCESS);
}





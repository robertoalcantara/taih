/* 
 * File:   main.c
 * Author: robertoalcantara
 *
 * Created on 24 de Abril de 2015, 16:00
 */

#include <stdio.h>
#include <stdlib.h>

#include "mcc_generated_files/mcc.h"
#include "globals.h"
#include "modem.h"


unsigned char rx_data_available = 0;
unsigned char rx_data[256];
unsigned int rx_data_index = 0;

unsigned char battery_level = 0;

/*
 *
 */
void setup (void) {

    SYSTEM_Initialize();
    
    INTERRUPT_GlobalInterruptEnable();
    INTERRUPT_PeripheralInterruptEnable();

    MODEM_PWR_SetLow();
    MODEM_PWR_SetHigh();
}




int start_modem(void) {
    static char state_modem = 0;

    switch (state_modem) {
        case 0:
            MODEM_PWR_SetLow();
            if ( global_timer.on1seg ) state_modem++;
            break;

        case 1:
            //aguarda em BAIXO
            MODEM_PWR_SetLow();
            if ( global_timer.on1seg ) state_modem++;
            break;

        case 2:
            MODEM_PWR_SetHigh();
            state_modem++;
    }

}




void serial_buffer_copy(void){
    char aux;

    if (rx_data_available) { /* Buffer overrun! Vai perder o dado. */ }

    while ( eusart1RxCount > 0  ) {
        aux = EUSART1_Read();

        if (aux == '\r') continue;
        rx_data[ rx_data_index ] = aux;
        rx_data_index++;
        if ( aux == '\n' ) {
            rx_data_available = 1;
            LED_D7_Toggle();
            break;
        }


    }

}


/*
 * 
 */
int main() {

    setup ();



    while (1) {

        start_modem(); //maquina de estado de configuracao do modem

       modem_state_machine();

       
        if ( global_timer.on1seg)
            LED_D6_Toggle();
        

        /* Verifica se existe dado na serial para processar */
        if ( eusart1RxCount > 0 ) {
            serial_buffer_copy();
        }
       // modem_async_parser(); //Ja analiza as mensagens assincronas

        /* flags de tempo */
        global_timer.on1seg  = 0;
        global_timer.on100ms = 0;
        global_timer.on10ms  = 0;
        global_timer.on1ms  = 0;
        while (global_timer.on1ms == 0) { /* Fazer nada */ }

    }

    return (EXIT_SUCCESS);
}





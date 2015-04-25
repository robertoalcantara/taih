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

#include "mcc_generated_files/mcc.h"
#include "mcc_generated_files/memory.h"
#include "globals.h"
#include "modem.h"



unsigned char rx_data_available = 0;
unsigned char rx_data[RX_BUFFER_SIZE];
unsigned int rx_data_index = 0;

unsigned char battery_level = 0;

unsigned char modem_power_status = 0;

/*
 *
 */
void setup (void) {

    SYSTEM_Initialize();
    
    INTERRUPT_GlobalInterruptEnable();
    INTERRUPT_PeripheralInterruptEnable();

    VBAT_SetAnalogMode();
    VBAT_CONTROL_SetLow();
}


void check_vbat(void){
    unsigned int vbat;
    
    vbat = VBAT_GetValue();
    if (vbat<200) {
        if ( global_timer.on1seg) {LED_D7_Toggle();}
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

    char cnt = 0;

    setup ();

    MODEM_ENABLE;
    
    while (1) {

        //if ( global_timer.on1seg) { check_vbat(); }
        
        if (0 == modem_handler() ) {
            /* Modem nao esta como deveria*/
            goto error;
        }

       
        if ( global_timer.on1seg) {
            //vivo
            LED_D6_Toggle();
        }
        

        /* Verifica se existe dado na serial para processar */
        if ( eusart1RxCount > 0 ) {
            serial_buffer_copy();
        }
       // modem_async_parser(); //Ja analiza as mensagens assincronas

        
error:
        /* flags de tempo */
        global_timer.on1seg  = 0;
        global_timer.on100ms = 0;
        global_timer.on10ms  = 0;
        global_timer.on1ms  = 0;
        while (global_timer.on1ms == 0) { /* Fazer nada */ }

    }

    return (EXIT_SUCCESS);
}





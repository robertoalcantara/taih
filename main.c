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
#include "globals.h"
#include "modem.h"


unsigned char rx_data_available = 0;
unsigned char rx_data[256];
unsigned int rx_data_index = 0;

unsigned char battery_level = 0;

unsigned long flash_pointer = 0x1000;

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

    ///////////////////////////////////


    /*flash_pointer =  0x1000;

    uint8_t wrBlockData[] =
    {
        0x00, 0x01, 0x02, 0x03, 0x04, 0x45, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
        0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F,
        0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2A, 0x2B, 0x2C, 0x2D, 0x2E, 0x2F,
        0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3A, 0x3B, 0x3C, 0x3D, 0x3E, 0x3F
    };

    // write to Flash memory block
    FLASH_WriteBlock((uint32_t)flash_pointer, (uint8_t *)wrBlockData);

    
    unsigned char t,z;

    t = FLASH_ReadWord(flash_pointer+5);
    z = FLASH_ReadWord(flash_pointer+6); */



    ////////////////////////////


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





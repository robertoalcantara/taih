/**
  @Generated Pin Manager Header File

  @Company:
    Microchip Technology Inc.

  @File Name:
    pin_manager.h

  @Summary:
    This is the Pin Manager file generated using MPLAB® Code Configurator

  @Description:
    This header file provides implementations for pin APIs for all pins selected in the GUI.
    Generation Information :
        Product Revision  :  MPLAB® Code Configurator - v2.10.3
        Device            :  PIC18F25K22
        Version           :  1.01
    The generated drivers are tested against the following:
        Compiler          :  XC8 v1.34
        MPLAB             :  MPLAB X 2.26
*/

/*
Copyright (c) 2013 - 2015 released Microchip Technology Inc.  All rights reserved.

Microchip licenses to you the right to use, modify, copy and distribute
Software only when embedded on a Microchip microcontroller or digital signal
controller that is integrated into your product or third party product
(pursuant to the sublicense terms in the accompanying license agreement).

You should refer to the license agreement accompanying this Software for
additional information regarding your rights and obligations.

SOFTWARE AND DOCUMENTATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY KIND,
EITHER EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION, ANY WARRANTY OF
MERCHANTABILITY, TITLE, NON-INFRINGEMENT AND FITNESS FOR A PARTICULAR PURPOSE.
IN NO EVENT SHALL MICROCHIP OR ITS LICENSORS BE LIABLE OR OBLIGATED UNDER
CONTRACT, NEGLIGENCE, STRICT LIABILITY, CONTRIBUTION, BREACH OF WARRANTY, OR
OTHER LEGAL EQUITABLE THEORY ANY DIRECT OR INDIRECT DAMAGES OR EXPENSES
INCLUDING BUT NOT LIMITED TO ANY INCIDENTAL, SPECIAL, INDIRECT, PUNITIVE OR
CONSEQUENTIAL DAMAGES, LOST PROFITS OR LOST DATA, COST OF PROCUREMENT OF
SUBSTITUTE GOODS, TECHNOLOGY, SERVICES, OR ANY CLAIMS BY THIRD PARTIES
(INCLUDING BUT NOT LIMITED TO ANY DEFENSE THEREOF), OR OTHER SIMILAR COSTS.
*/

#ifndef PIN_MANAGER_H
#define PIN_MANAGER_H

#define INPUT   1
#define OUTPUT  0

#define HIGH    1
#define LOW     0

#define ANALOG      1
#define DIGITAL     0

#define PULL_UP_ENABLED      1
#define PULL_UP_DISABLED     0

// get/set NETPIN aliases
#define NETPIN_TRIS               TRISA0
#define NETPIN_LAT                LATA0
#define NETPIN_PORT               PORTAbits.RA0
#define NETPIN_ANS                ANSA0
#define NETPIN_SetHigh()    do { LATA0 = 1; } while(0)
#define NETPIN_SetLow()   do { LATA0 = 0; } while(0)
#define NETPIN_Toggle()   do { LATA0 = ~LATA0; } while(0)
#define NETPIN_GetValue()         PORTAbits.RA0
#define NETPIN_SetDigitalInput()    do { TRISA0 = 1; } while(0)
#define NETPIN_SetDigitalOutput()   do { TRISA0 = 0; } while(0)

#define NETPIN_SetAnalogMode()   do { ANSA0 = 1; } while(0)
#define NETPIN_SetDigitalMode()   do { ANSA0 = 0; } while(0)
// get/set VBAT_CONTROL aliases
#define VBAT_CONTROL_TRIS               TRISA4
#define VBAT_CONTROL_LAT                LATA4
#define VBAT_CONTROL_PORT               PORTAbits.RA4
#define VBAT_CONTROL_SetHigh()    do { LATA4 = 1; } while(0)
#define VBAT_CONTROL_SetLow()   do { LATA4 = 0; } while(0)
#define VBAT_CONTROL_Toggle()   do { LATA4 = ~LATA4; } while(0)
#define VBAT_CONTROL_GetValue()         PORTAbits.RA4
#define VBAT_CONTROL_SetDigitalInput()    do { TRISA4 = 1; } while(0)
#define VBAT_CONTROL_SetDigitalOutput()   do { TRISA4 = 0; } while(0)

// get/set channel_AN4 aliases
#define channel_AN4_TRIS               TRISA5
#define channel_AN4_LAT                LATA5
#define channel_AN4_PORT               PORTAbits.RA5
#define channel_AN4_ANS                ANSA5
#define channel_AN4_SetHigh()    do { LATA5 = 1; } while(0)
#define channel_AN4_SetLow()   do { LATA5 = 0; } while(0)
#define channel_AN4_Toggle()   do { LATA5 = ~LATA5; } while(0)
#define channel_AN4_GetValue()         PORTAbits.RA5
#define channel_AN4_SetDigitalInput()    do { TRISA5 = 1; } while(0)
#define channel_AN4_SetDigitalOutput()   do { TRISA5 = 0; } while(0)

#define channel_AN4_SetAnalogMode()   do { ANSA5 = 1; } while(0)
#define channel_AN4_SetDigitalMode()   do { ANSA5 = 0; } while(0)
// get/set LED_D6 aliases
#define LED_D6_TRIS               TRISA6
#define LED_D6_LAT                LATA6
#define LED_D6_PORT               PORTAbits.RA6
#define LED_D6_SetHigh()    do { LATA6 = 1; } while(0)
#define LED_D6_SetLow()   do { LATA6 = 0; } while(0)
#define LED_D6_Toggle()   do { LATA6 = ~LATA6; } while(0)
#define LED_D6_GetValue()         PORTAbits.RA6
#define LED_D6_SetDigitalInput()    do { TRISA6 = 1; } while(0)
#define LED_D6_SetDigitalOutput()   do { TRISA6 = 0; } while(0)

// get/set LED_D7 aliases
#define LED_D7_TRIS               TRISA7
#define LED_D7_LAT                LATA7
#define LED_D7_PORT               PORTAbits.RA7
#define LED_D7_SetHigh()    do { LATA7 = 1; } while(0)
#define LED_D7_SetLow()   do { LATA7 = 0; } while(0)
#define LED_D7_Toggle()   do { LATA7 = ~LATA7; } while(0)
#define LED_D7_GetValue()         PORTAbits.RA7
#define LED_D7_SetDigitalInput()    do { TRISA7 = 1; } while(0)
#define LED_D7_SetDigitalOutput()   do { TRISA7 = 0; } while(0)

// get/set BTN aliases
#define BTN_TRIS               TRISB0
#define BTN_LAT                LATB0
#define BTN_PORT               PORTBbits.RB0
#define BTN_WPU                WPUB0
#define BTN_ANS                ANSB0
#define BTN_SetHigh()    do { LATB0 = 1; } while(0)
#define BTN_SetLow()   do { LATB0 = 0; } while(0)
#define BTN_Toggle()   do { LATB0 = ~LATB0; } while(0)
#define BTN_GetValue()         PORTBbits.RB0
#define BTN_SetDigitalInput()    do { TRISB0 = 1; } while(0)
#define BTN_SetDigitalOutput()   do { TRISB0 = 0; } while(0)

#define BTN_SetPullup()    do { WPUB0 = 1; } while(0)
#define BTN_ResetPullup()   do { WPUB0 = 0; } while(0)
#define BTN_SetAnalogMode()   do { ANSB0 = 1; } while(0)
#define BTN_SetDigitalMode()   do { ANSB0 = 0; } while(0)
// get/set PWR_STAT aliases
#define PWR_STAT_TRIS               TRISB5
#define PWR_STAT_LAT                LATB5
#define PWR_STAT_PORT               PORTBbits.RB5
#define PWR_STAT_WPU                WPUB5
#define PWR_STAT_ANS                ANSB5
#define PWR_STAT_SetHigh()    do { LATB5 = 1; } while(0)
#define PWR_STAT_SetLow()   do { LATB5 = 0; } while(0)
#define PWR_STAT_Toggle()   do { LATB5 = ~LATB5; } while(0)
#define PWR_STAT_GetValue()         PORTBbits.RB5
#define PWR_STAT_SetDigitalInput()    do { TRISB5 = 1; } while(0)
#define PWR_STAT_SetDigitalOutput()   do { TRISB5 = 0; } while(0)

#define PWR_STAT_SetPullup()    do { WPUB5 = 1; } while(0)
#define PWR_STAT_ResetPullup()   do { WPUB5 = 0; } while(0)
#define PWR_STAT_SetAnalogMode()   do { ANSB5 = 1; } while(0)
#define PWR_STAT_SetDigitalMode()   do { ANSB5 = 0; } while(0)
// get/set TX2 aliases
#define TX2_TRIS               TRISB6
#define TX2_LAT                LATB6
#define TX2_PORT               PORTBbits.RB6
#define TX2_WPU                WPUB6
#define TX2_SetHigh()    do { LATB6 = 1; } while(0)
#define TX2_SetLow()   do { LATB6 = 0; } while(0)
#define TX2_Toggle()   do { LATB6 = ~LATB6; } while(0)
#define TX2_GetValue()         PORTBbits.RB6
#define TX2_SetDigitalInput()    do { TRISB6 = 1; } while(0)
#define TX2_SetDigitalOutput()   do { TRISB6 = 0; } while(0)

#define TX2_SetPullup()    do { WPUB6 = 1; } while(0)
#define TX2_ResetPullup()   do { WPUB6 = 0; } while(0)
// get/set RX2 aliases
#define RX2_TRIS               TRISB7
#define RX2_LAT                LATB7
#define RX2_PORT               PORTBbits.RB7
#define RX2_WPU                WPUB7
#define RX2_SetHigh()    do { LATB7 = 1; } while(0)
#define RX2_SetLow()   do { LATB7 = 0; } while(0)
#define RX2_Toggle()   do { LATB7 = ~LATB7; } while(0)
#define RX2_GetValue()         PORTBbits.RB7
#define RX2_SetDigitalInput()    do { TRISB7 = 1; } while(0)
#define RX2_SetDigitalOutput()   do { TRISB7 = 0; } while(0)

#define RX2_SetPullup()    do { WPUB7 = 1; } while(0)
#define RX2_ResetPullup()   do { WPUB7 = 0; } while(0)
// get/set MODEM_PWR aliases
#define MODEM_PWR_TRIS               TRISC0
#define MODEM_PWR_LAT                LATC0
#define MODEM_PWR_PORT               PORTCbits.RC0
#define MODEM_PWR_SetHigh()    do { LATC0 = 1; } while(0)
#define MODEM_PWR_SetLow()   do { LATC0 = 0; } while(0)
#define MODEM_PWR_Toggle()   do { LATC0 = ~LATC0; } while(0)
#define MODEM_PWR_GetValue()         PORTCbits.RC0
#define MODEM_PWR_SetDigitalInput()    do { TRISC0 = 1; } while(0)
#define MODEM_PWR_SetDigitalOutput()   do { TRISC0 = 0; } while(0)

// get/set TX1 aliases
#define TX1_TRIS               TRISC6
#define TX1_LAT                LATC6
#define TX1_PORT               PORTCbits.RC6
#define TX1_ANS                ANSC6
#define TX1_SetHigh()    do { LATC6 = 1; } while(0)
#define TX1_SetLow()   do { LATC6 = 0; } while(0)
#define TX1_Toggle()   do { LATC6 = ~LATC6; } while(0)
#define TX1_GetValue()         PORTCbits.RC6
#define TX1_SetDigitalInput()    do { TRISC6 = 1; } while(0)
#define TX1_SetDigitalOutput()   do { TRISC6 = 0; } while(0)

#define TX1_SetAnalogMode()   do { ANSC6 = 1; } while(0)
#define TX1_SetDigitalMode()   do { ANSC6 = 0; } while(0)
// get/set RX1 aliases
#define RX1_TRIS               TRISC7
#define RX1_LAT                LATC7
#define RX1_PORT               PORTCbits.RC7
#define RX1_ANS                ANSC7
#define RX1_SetHigh()    do { LATC7 = 1; } while(0)
#define RX1_SetLow()   do { LATC7 = 0; } while(0)
#define RX1_Toggle()   do { LATC7 = ~LATC7; } while(0)
#define RX1_GetValue()         PORTCbits.RC7
#define RX1_SetDigitalInput()    do { TRISC7 = 1; } while(0)
#define RX1_SetDigitalOutput()   do { TRISC7 = 0; } while(0)

#define RX1_SetAnalogMode()   do { ANSC7 = 1; } while(0)
#define RX1_SetDigitalMode()   do { ANSC7 = 0; } while(0)

/**
 * @Param
    none
 * @Returns
    none
 * @Description
    GPIO and peripheral I/O initialization
 * @Example
    PIN_MANAGER_Initialize();
 */
void PIN_MANAGER_Initialize (void);

/**
 * @Param
    none
 * @Returns
    none
 * @Description
    Interrupt on Change Handling routine
 * @Example
    PIN_MANAGER_IOC();
 */
void PIN_MANAGER_IOC(void);

#endif // PIN_MANAGER_H
/**
 End of File
*/
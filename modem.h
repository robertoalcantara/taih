/* 
 * File:   modem.h
 * Author: robertoalcantara
 *
 * Created on 25 de Mar�o de 2015, 11:01
 */

#ifndef MODEM_H
#define	MODEM_H

#define MODEM_ENABLE modem_power_status = 1; 
#define MODEM_DISABLE modem_power_status = 0; state_modem = 0;

extern unsigned char modem_setup();
extern unsigned char state_modem;

extern unsigned char state_main; //maquina de estados principal

extern void modem_async_parser();

extern unsigned char modem_handler();


extern int power_modem( char enable );


#endif	/* MODEM_H */


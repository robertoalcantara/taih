/* 
 * File:   modem.h
 * Author: robertoalcantara
 *
 * Created on 25 de Março de 2015, 11:01
 */

#ifndef MODEM_H
#define	MODEM_H

#define MODEM_ENABLE modem_power_status = 1;
#define MODEM_DISABLE modem_power_status = 0;

unsigned char modem_setup();

void modem_async_parser();

unsigned char modem_handler();


#endif	/* MODEM_H */


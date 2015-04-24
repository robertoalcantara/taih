/* 
 * File:   modem.h
 * Author: robertoalcantara
 *
 * Created on 25 de Março de 2015, 11:01
 */

#ifndef MODEM_H
#define	MODEM_H

unsigned char modem_setup();

void modem_async_parser();

unsigned char modem_state_machine();


#endif	/* MODEM_H */


/* 
 * File:   globals.h
 * Author: robertoalcantara
 *
 * Created on 25 de Março de 2015, 14:43
 */





#ifndef GLOBALS_H

#define	GLOBALS_H
#define RX_LEN 256


#define ERASE_FLASH_BLOCKSIZE 64

extern unsigned char rx_data_available;
extern unsigned char rx_data[RX_LEN];
extern unsigned int rx_data_index;

extern unsigned char battery_level;


struct t_global_timer {
      char on1seg;
      char on100ms;
      char on10ms;
      char on1ms;
      
      unsigned char aux_10ms;
      unsigned char aux_100ms;
      unsigned char aux_1s;

} global_timer ;


int x;
#define RX_DATA_ACK rx_data_index = 0; rx_data_available = 0; rx_data[0]=0; for (x=0;x<RX_LEN;x++) rx_data[x]=0;

extern unsigned long flash_pointer;



#endif	/* GLOBALS_H */


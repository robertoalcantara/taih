/* 
 * File:   globals.h
 * Author: robertoalcantara
 *
 * Created on 25 de Março de 2015, 14:43
 */


#ifndef GLOBALS_H
#define	GLOBALS_H

#define _XTAL_FREQ 2000000

#define RX_BUFFER_SIZE 512

#define SINALIZACAO_NORMAL 0x00
#define SINALIZACAO_MODEM_FAULT 0x01
#define SINALIZACAO_SIM_FAULT 0x02
#define SINALIZACAO_MSG_ACK 0x10


#define SINALIZA_NORMAL sinalizacao_status = SINALIZACAO_NORMAL;
#define SINALIZA_MODEM_FAULT sinalizacao_status =  SINALIZACAO_MODEM_FAULT;
#define SINALIZA_SIM_FAULT sinalizacao_status = SINALIZACAO_SIM_FAULT;

#define SINALIZA_NADA sinalizacao_status = 99; //qdo em bateria baixa desliga a sinalizacao


#define SINALIZA_MSG_ACK    sinalizacao_status = sinalizacao_status | (0xF0 & SINALIZACAO_MSG_ACK);

#define LOW_BATTERY 10

#define ERASE_FLASH_BLOCKSIZE 64

#define SUCCESS 200 /* Final da maquina de "estado com sucesso*/


#define TIMEOUT_STATE_MODEM 350

#define TIMEOUT_PERMANENT_MODEM TIMEOUT_STATE_MODEM*4

extern unsigned long vbat;
extern unsigned char rx_data_available;
extern unsigned char rx_data[RX_BUFFER_SIZE];
extern unsigned int rx_data_index;

extern unsigned char battery_level;

unsigned char modem_power_status;

extern unsigned char sinalizacao_status;


typedef struct t_global_timer {
      char on1seg;
      char on100ms;
      char on10ms;
      char on1ms;
      
      unsigned char aux_10ms;
      unsigned char aux_100ms;

} T_GLOBAL_TIMER ;

extern volatile T_GLOBAL_TIMER global_timer;

int x;
#define RX_DATA_ACK rx_data_index = 0; rx_data_available = 0; rx_data[0]=0; //for (x=0;x<RX_BUFFER_SIZE;x++) rx_data[x]=0;

extern unsigned long flash_pointer;


extern void printD(const char* str);

#endif	/* GLOBALS_H */


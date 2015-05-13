/* 
 * File:   flash_io.h
 * Author: robertoalcantara
 *
 * Created on 25 de Abril de 2015, 17:43
 */

#ifndef FLASH_IO_H
#define	FLASH_IO_H

#define BASE_ADDR 0x3000
#define BLOCK_SIZE ERASE_FLASH_BLOCKSIZE

#define RESET_FLASH flashAddr = BASE_ADDR;

extern uint32_t   flashAddr;

extern void flash_write_char(unsigned char ch);

extern void flash_commit(void);



#endif	/* FLASH_IO_H */


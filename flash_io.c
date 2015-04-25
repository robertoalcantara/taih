/* 
 * File:   flash_io.c
 * Author: robertoalcantara
 *
 * Created on 25 de Abril de 2015, 17:31
 */

#include <stdio.h>
#include <stdlib.h>
#include "mcc_generated_files/memory.h"

/*
 * 
 */
#define BASE_ADDR 0x3000
#define BLOCK_SIZE ERASE_FLASH_BLOCKSIZE

uint32_t   flashAddr = BASE_ADDR;  //ponteiro para endereco
uint8_t    flashBuf[BLOCK_SIZE];

unsigned char idx_current_block;


void flash_write_char(unsigned char ch) {
 
    flashBuf[ idx_current_block ] = ch;
    idx_current_block++;

    if (idx_current_block >= BLOCK_SIZE ) {
        /* Completou um bloco */
        FLASH_WriteBlock(flashAddr, (uint8_t *)flashBuf);
        idx_current_block = 0;
        flashAddr += BLOCK_SIZE;
    }
}

void flash_commit(void) {
    //Completa o bloco com \0 e garante a gravacao
    while ( idx_current_block < BLOCK_SIZE ) {
        flashBuf[ idx_current_block ] = 0;
        idx_current_block++;
    }
    FLASH_WriteBlock(flashAddr, (uint8_t *)flashBuf);
    idx_current_block = 0;
    flashAddr += BLOCK_SIZE;
}



/* 
 * File:   expect.h
 * Author: robertoalcantara
 *
 * Created on 25 de Março de 2015, 10:11
 */

#ifndef EXPECT_H
#define	EXPECT_H

#define EXPECT_TIMEOUT 0
#define EXPECT_WAITING 1
#define EXPECT_FOUND 2

#define EXPECT_ERROR init_flag=0;

extern unsigned char init_flag;

unsigned char expect( char* source, char* target, unsigned char timeout, char flag_change );

#endif	/* EXPECT_H */


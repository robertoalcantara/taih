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


#define DEFAULT_EXPECT_TIMEOUT 5

#define EXPECT_ERROR timeout_expect_count = DEFAULT_EXPECT_TIMEOUT;

extern unsigned char timeout_expect_count;

unsigned char expect( char* source, char* target,  char flag_change );
void set_expect_timeout( unsigned char timeout );


#endif	/* EXPECT_H */


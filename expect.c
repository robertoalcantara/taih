

/*
 * Global 1s tick
 */

#include <string.h> 
#include "expect.h" 
#include "globals.h"
#include <stdio.h>

unsigned char timeout_count = 0;
unsigned char init_flag = 0;

/* recebe a string de origem (buffer), o que estamos procurando (target), timeout em segundos e o flag que indica que houve mudanca a ser processada */

unsigned char expect( char* source, char* target, unsigned char timeout, char flag_change   ) {
   

    if ( 0 == init_flag ) {
        init_flag = 1; /* blocking timeout count value*/
        timeout_count = timeout;
    }

    if ( global_timer.on1seg ) { 
        timeout_count --;

    }
    if ( 0 == timeout_count ) {
        /* Not found */
        init_flag = 0;
    
        return EXPECT_TIMEOUT;
    }

    if (flag_change) {
    
        if ( strstr(source, target) ) {
            /* Found */
            init_flag = 0;
            return EXPECT_FOUND;
        }
    }
    /* Keep waiting */
    return EXPECT_WAITING;
}




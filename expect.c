

/*
 * Global 1s tick
 */

#include <string.h> 
#include "expect.h" 
#include "globals.h"
#include <stdio.h>
#include "mcc_generated_files/mcc.h"


unsigned char timeout_expect_count = 5;

void set_expect_timeout( unsigned char timeout ) {
        timeout_expect_count = timeout;
}

/* recebe a string de origem (buffer), o que estamos procurando (target), timeout em segundos e o flag que indica que houve mudanca a ser processada */

unsigned char expect( char* source, char* target, char flag_change   ) {


    if ( global_timer.on1seg  ) {
        timeout_expect_count --;
    }
    if ( timeout_expect_count <= 0 ) {
        /* Not found */
        timeout_expect_count = DEFAULT_EXPECT_TIMEOUT;
    
        return EXPECT_TIMEOUT;
    }

    if (flag_change) {
    
        if ( strstr(source, target) ) {
            /* Found */
            timeout_expect_count = DEFAULT_EXPECT_TIMEOUT;
            return EXPECT_FOUND;
        }
    }
    /* Keep waiting */
    return EXPECT_WAITING;
}




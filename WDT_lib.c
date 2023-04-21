/*
 * File:   WDT_lib.c
 * Author: magnu
 *
 * Created on April 20, 2023, 7:55 PM
 */


#include "xc.h"
#include "WDT_lib.h"


void sleepNperiods(int N) {
    int t;
    for(t = 0; t < N; t++) {
        RCONbits.SWDTEN = 1;
        Sleep();
        RCONbits.SLEEP  = 0;
        RCONbits.WDTO = 0;
        RCONbits.SWDTEN = 0;
    }
    return;
}
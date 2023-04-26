/*
 * File:   WDT_lib.c
 * Author: GardenBot Team
 * Purpose: 
 * Created on April 20, 2023, 7:55 PM
 */


#include "xc.h"
#include "WDT_lib.h"

/* This function allows the PIC to go to sleep for periods longer than 1 
 * WDT period. This is implemented by counting a variable up to N, increasing
 * time the PIC wakes from sleep. Total sleep time will be N * WDT period
 * 
 * Arguments: int N - the number of desired periods to sleep for
 * Returns: nothing
 */
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
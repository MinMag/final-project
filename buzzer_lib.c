#include "xc.h"
#include "WDT_lib.h"
#include "ADC_lib.h"
#include <p24FJ64GA002.h>

#define WATERLEVELTHRESHOLD (1.65/3.3)*1023 //ADC2, Threshold of 1.65

/* Enables the RB6 pin
 */
void buzzerEnable(){
    LATBbits.LATB6 = 1; //sets RB6 as high
}

/* Keeps buzzer on for 10 cycles of timer2, which is equal to 10 seconds or until the water level surpasses the threshold
 */
void buzzerDisable(volatile int* overflow){
    *overflow = 0;
    while(*overflow < 10 && getAvgWaterLevel() < WATERLEVELTHRESHOLD); //Keeps buzzer on for 10 seconds or until the water level passes the threshold
    LATBbits.LATB6 = 0; // sets RB6 as low
}

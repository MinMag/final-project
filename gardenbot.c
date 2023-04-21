/*
 * File:   newmainXC16.c
 * Author: magnu
 *
 * Created on April 7, 2023, 2:39 PM
 */
#include <p24FJ64GA002.h>

#include "xc.h"
// CW1: FLASH CONFIGURATION WORD 1 (see PIC24 Family Reference Manual 24.1)
#pragma config ICS = PGx1          // Comm Channel Select (Emulator EMUC1/EMUD1 pins are shared with PGC1/PGD1)
#pragma config FWDTEN = OFF        // Watchdog Timer Enable (Watchdog Timer is disabled)
#pragma config GWRP = OFF          // General Code Segment Write Protect (Writes to program memory are allowed)
#pragma config GCP = OFF           // General Code Segment Code Protect (Code protection is disabled)
#pragma config JTAGEN = OFF        // JTAG Port Enable (JTAG port is disabled)



#pragma config FWPSA = PR128       // Configures Prescalar for WDT, in this case 
#pragma config WDTPS = PS32768     // Configures Postscalar for WDT
                                   // WDT period (ms) = (FWPSA/32) * WDTPS


// CW2: FLASH CONFIGURATION WORD 2 (see PIC24 Family Reference Manual 24.1)
#pragma config I2C1SEL = PRI       // I2C1 Pin Location Select (Use default SCL1/SDA1 pins)
#pragma config IOL1WAY = OFF       // IOLOCK Protection (IOLOCK may be changed via unlocking seq)
#pragma config OSCIOFNC = ON       // Primary Oscillator I/O Function (CLKO/RC15 functions as I/O pin)
#pragma config FCKSM = CSECME      // Clock Switching and Monitor (Clock switching is enabled, 
                                       // Fail-Safe Clock Monitor is enabled)
#pragma config FNOSC = FRCPLL      // Oscillator Select (Fast RC Oscillator with PLL module (FRCPLL))

void delay_ms(unsigned int ms) {
    while (ms-- > 0) {
        asm("repeat #15998");
        asm("nop");
    }
}

int main(void) {
    _RCDIV = 2;
    AD1PCFG = 0xffff;
    TRISBbits.TRISB6 = 0;    
    
    while(1) {
        LATBbits.LATB6 = 0;
        delay_ms(1000);
        LATBbits.LATB6 = 1;
        
//        ClrWdt();
        int x = 0;
        //Below is basic structure to create a desired sleep longer than one WDT period
        while(x < 2) { //Sleep for approximately 2 * WDTPeriod seconds w/ the LED on
            RCONbits.SWDTEN = 1;
            Sleep();
            RCONbits.SLEEP  = 0;
            RCONbits.WDTO = 0;
            RCONbits.SWDTEN = 0;
            x++;
        }
    }
    
    
    
    return 0;
}

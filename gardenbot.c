#include "xc.h"
#include "WDT_lib.h"
#include "ADC_lib.h"
#include <p24FJ64GA002.h>

#pragma config ICS = PGx1
#pragma config FWDTEN = OFF    
#pragma config GWRP = OFF   	 
#pragma config GCP = OFF    	 
#pragma config JTAGEN = OFF	 
#pragma config I2C1SEL = PRI	 
#pragma config IOL1WAY = OFF 	 
#pragma config OSCIOFNC = ON 	 
#pragma config FCKSM = CSECME    
#pragma config FNOSC = FRCPLL
#pragma config FWPSA = PR32       // Configures Prescalar for WDT, in this case 
#pragma config WDTPS = PS1024     // Configures Postscalar for WDT
                                   // WDT period (ms) = (FWPSA/32) * WDTPS
 
#define MOISTURETHRESHOLD (1.8/3.3)*1023 //ADC1, I think this is for a threshold of 1.8V on 0-3.3 scale
#define WATERLEVELTHRESHOLD (1.65/3.3)*1023 //ADC2

volatile int overflow = 0;

void initPushButton(void) {
	__builtin_write_OSCCONL(OSCCON & 0xbf); // unlock PPS
	RPINR7bits.IC1R = 8; // Use Pin RP8 = "8", for Input Capture 1 (Table 10-2)
	__builtin_write_OSCCONL(OSCCON | 0x40); // lock   PPS
	T2CON = 0;
	T2CONbits.TCKPS = 0b11;
	PR2 = 62499; // 1s cycle
	TMR2 = 0;
	IEC0bits.T2IE = 1; // Enable T2 Interrupt
	TRISBbits.TRISB8 = 1; // Pin 17 is input
	CNPU2bits.CN22PUE = 1; //Pull-up Resistor Enable
	_T2IF = 0;
	T2CONbits.TON = 0;
	IC1CON = 0;
	IC1CONbits.ICTMR = 1;
	IC1CONbits.ICM = 0b010; // Capture every fall
	IC1BUF = 0;
	IEC0bits.IC1IE = 1; // Input capture 1 Interrupt Enable
}
void pumpEnable(){
    	LATBbits.LATB12 = 1;	// Set RB12 high
    	delay_ms(1000);
}
void pumpDisable(){
        LATBbits.LATB12 = 0;	// Set RB12 low
    	LATBbits.LATB12 = 0;	// Set RB12 low
}
void __attribute__((interrupt, auto_psv)) _T2Interrupt() { // rollover for T2 ISR
	_T2IF = 0;
	TMR2 = 0; 
    overflow +=1;
}
void __attribute__((interrupt, auto_psv)) _IC1Interrupt() { // Detect click ISR
	_IC1IF = 0;
	pumpEnable();
	}   
void buzzerEnable(){
    LATBbits.LATB6 = 1; //sets RB6 as high
    overflow = 0;
    while(overflow < 10 && getAvg1() < WATERLEVELTHRESHOLD);
    LATBbits.LATB6 = 0; // sets RB6 as low
}
void loop() {
	while (1) {
        delay_ms(1000);
//    	while (IFS0bits.T1IF == 0);
//        IFS0bits.T1IF = 0;
     	if(getAvg2() < WATERLEVELTHRESHOLD){
            buzzerEnable();
            sleepNperiods(2); //Waiting 4 minutes before we 
        	//Buzzer
        	//Buzz more
    	}
    	else{
        	if(getAvg1() > MOISTURETHRESHOLD){
            	pumpEnable(); //Water soil
            	//Wait 5 minutes
                sleepNperiods(2); //Waiting ~4mins, 1 WDT period is 131 seconds approx.
        	}
        	else{
            	//Go to sleep
                sleepNperiods(2); //Wait in sleep for approx. 30 minutes
               
//                TMR1 = 0;
//                T1CONbits.TON = 1;
//                while(IFS0bits.T1IF ==0);
//                IFS0bits.T1IF = 0;
//                T1CONbits.TON = 0;
        	}
    	}
   	 
   
	}
}
int main() {
	initBuffer();
	pic24_setup();
	initPushButton();
	adc_init1();
	T2CONbits.TON = 1;
    
    // Turn on RB12 and RB6 in a startup rhythm
    LATBbits.LATB12 = 1;   // Turn on RB12
    delay_ms(100);         // Delay for 100 ms
    LATBbits.LATB6 = 1;    // Turn on RB6
    delay_ms(50);          // Delay for 50 ms
    LATBbits.LATB12 = 0;   // Turn off RB12
    delay_ms(50);          // Delay for 50 ms
    LATBbits.LATB6 = 0;    // Turn off RB6
    delay_ms(100);         // Delay for 100 ms

    LATBbits.LATB6 = 1;    // Turn on RB6
    delay_ms(100);         // Delay for 100 ms
    LATBbits.LATB12 = 1;   // Turn on RB12
    delay_ms(50);          // Delay for 50 ms
    LATBbits.LATB6 = 0;    // Turn off RB6
    delay_ms(50);          // Delay for 50 ms
    LATBbits.LATB12 = 0;   // Turn off RB12

    loop();
    
}
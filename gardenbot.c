#include "xc.h"
#include "WDT_lib.h"
#include "ADC_lib.h"
#include "buzzer_lib.h"
#include <p24FJ64GA002.h>

#pragma config ICS = PGx1
#pragma config FWDTEN = OFF //This must be configured to OFF to allow for proper
                            //sleep functionality
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
 
#define MOISTURETHRESHOLD (1.95/3.3)*1023 //ADC2, Threshold of 1.8V
#define WATERLEVELTHRESHOLD (1.1/3.3)*1023 //ADC1, Threshold of 1.65V
#define PERIODSAFTERWATER 2
#define PERIODSAFTERLOWWATER 2
#define PERIODSAFTERNOTHING 2

volatile int overflow = 0; //A variable to allow for early exit from
                           //buzzerDisable() function

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
void __attribute__((interrupt, auto_psv)) _T2Interrupt() { // rollover for T2 ISR
	_T2IF = 0;
	TMR2 = 0; 
    overflow +=1; //This variable is used in the buzzerDisable function
}
void __attribute__((interrupt, auto_psv)) _IC1Interrupt() { // Detect click ISR
	_IC1IF = 0;
	pumpEnable();
    pumpDisable();
	}   

void loop() {
	while (1) {
       /* This blocking delay gives the ADC enough time
        * to ensure we are only acting on data captured
        * during this active period
        */
        delay_ms(1000); 
        //Water Level Voltage is Low when Dry
     	if(getAvgWaterLevel() < WATERLEVELTHRESHOLD){ 
            buzzerEnable();
            buzzerDisable(&overflow);
            //Sleeping 8 seconds before we continue
            sleepNperiods(PERIODSAFTERLOWWATER); 
    	}
        //Moisture Voltage is High when Dry
    	else if(getAvgMoisture() > MOISTURETHRESHOLD){
                //Water soil
            	pumpEnable(); 
                pumpDisable();
                //Sleeping 8 seconds, 1 WDT period is 4 seconds approx.
                sleepNperiods(PERIODSAFTERWATER); 
        	}
        else {
            	//Nothing is needed to be done in this active period since
                //the water level is high and the soil is moist
                //Sleeping in sleep for approx. 8 seconds
                sleepNperiods(PERIODSAFTERNOTHING); 
        	}
	}
}
int main() {
	initBuffer();
	pic24_setup();
	initPushButton();
	adc_init1();
	T2CONbits.TON = 1;
    
    loop();
    
}

#include "xc.h"
#include "WDT_lib.h"
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
#pragma config FWPSA = PR128       // Configures Prescalar for WDT, in this case 
#pragma config WDTPS = PS32768     // Configures Postscalar for WDT
                                   // WDT period (ms) = (FWPSA/32) * WDTPS

#define BUFSIZE 1024
#define NUMSAMPLES 30
#define MOISTURETHRESHOLD (1.8/3.3)*1023 //ADC1, I think this is for a threshold of 1.8V on 0-3.3 scale
#define WATERLEVELTHRESHOLD (1.65/3.3)*1023 //ADC2

unsigned int adc_buffer1[BUFSIZE];
unsigned int adc_buffer2[BUFSIZE];
volatile int buffer_index1 = 0;
volatile int buffer_index2 = 0;

volatile int state = 0;

void delay_ms(unsigned int ms) {
	while (ms-- > 0) {
    	asm("repeat #15998");
    	asm("nop");
	}
}
void pic24_setup(void){
	_RCDIV = 0;
	AD1PCFG = 0x7FFE;
	AD1PCFGbits.PCFG0 = 0; // set AN0 to analog
	AD1PCFGbits.PCFG1 = 0; // set AN1 to analog
	TRISBbits.TRISB12 = 0;   // Set RB12 as output
	TRISAbits.TRISA0 = 1;  // Set RA0/AN0 to Input
	TRISAbits.TRISA1 = 1; // Set RA1/AN1 to Input
}
unsigned int getAvg(unsigned int buffer[], int buffer_index){
	unsigned long sum = 0;
    	int index = buffer_index;
    	for (int i = 0; i < NUMSAMPLES; i++) {
        	sum += buffer[index--]; // use decrementing index
        	if (index < 0) { // check for buffer underflow
            	index = BUFSIZE - 1;
        	}
    	}
    	unsigned int average = sum / NUMSAMPLES;
    	return average;
}
void putVal(unsigned int ADCvalue, unsigned int buffer[], int buffer_index){
	buffer[buffer_index++] = ADCvalue;
	if(buffer_index >= BUFSIZE){
    	buffer_index = 0;
	}
}
void initBuffer(){
	int i;
	for(i=0; i < BUFSIZE; i++){
    	adc_buffer1[i] = 0;
    	adc_buffer2[i] = 0;
	}
}
void adc_init1(){
	AD1CON2bits.VCFG = 0b000; // VDD max (3.3V) and VSS min (0V)
    
	AD1CON3bits.ADCS = 1; // TAD >= 75 ns (Tcy = 62.5 ns)
	AD1CON1bits.SSRC = 2; // Sample on timer3 events
	AD1CON3bits.SAMC = 1; // At least 1 auto sample time bit
	AD1CON1bits.FORM = 0; // Unsigned int
	AD1CON1bits.ASAM = 1; // Reference Manual
	AD1CON2bits.SMPI = 0; // Reference Manual
	AD1CON1bits.ADON = 1; // Turn on ADC
    
	_AD1IF = 0;
	_AD1IE = 1; // Enable ADC interrupt
    
	TMR3 = 0;  // Set up timer3
	T3CON = 0;
	T3CONbits.TCKPS = 0b10;// 1:64
	PR3 = 15624;
	T3CONbits.TON = 1; // 16 times per second
}
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
void __attribute__((interrupt, auto_psv)) _ADC1Interrupt(void){
	putVal(ADC1BUF0, adc_buffer1, buffer_index1); // Call putVal() on adc_buffer1 with ADC1BUF0
	IFS0bits.AD1IF = 0; // Reset the ADC interrupt flag
}
void __attribute__((interrupt, auto_psv)) _ADC2Interrupt(void){
    putVal(ADC1BUF1, adc_buffer2, buffer_index2); // Call putVal() on adc_buffer2 with ADC1BUF1
    IFS0bits.AD1IF = 0; // Reset the ADC interrupt flag
}
void __attribute__((interrupt, auto_psv)) _T2Interrupt() { // rollover for T2 ISR
	_T2IF = 0;
	TMR2 = 0; 
}
void __attribute__((interrupt, auto_psv)) _IC1Interrupt() { // Detect click ISR
	_IC1IF = 0;
	pumpEnable();
	}   
void pumpEnable(){
    	LATBbits.LATB12 = 1;	// Set RB2 high
    	delay_ms(1000);
    	LATBbits.LATB12 = 0;	// Set RB2 low
    	state = 0;
}

void loop() {
	while (1) {
    	while (IFS0bits.T1IF == 0);
        IFS0bits.T1IF = 0;
    	if(getAvg(adc_buffer2, buffer_index2) < WATERLEVELTHRESHOLD){
        	//Buzzer
        	//Buzz more
    	}
    	else{
        	if(getAvg(adc_buffer1, buffer_index1) < MOISTURETHRESHOLD){
            	pumpEnable(); //Water soil
            	//Wait 5 minutes
                sleepNperiods(2); //Waiting ~4mins, 1 WDT period is 131 seconds approx.
        	}
        	else{
            	//Go to sleep
                sleepNperiods(14); //Wait in sleep for approx. 30 minutes
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
    
	loop();
    
}

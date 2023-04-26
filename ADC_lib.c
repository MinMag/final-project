#include "xc.h"
#include "WDT_lib.h"
#include "ADC_lib.h"
#include <p24FJ64GA002.h>

#define BUFSIZE 1024
#define NUMSAMPLES 30
#define INITWATERLEVEL 1000

unsigned int adc_buffer1[BUFSIZE];
unsigned int adc_buffer2[BUFSIZE];
volatile int buffer_index1 = 0;
volatile int buffer_index2 = 0;

/* Adapted ASM Code to delay milliseconds by an integer parameter */
void delay_ms(unsigned int ms) {
	while (ms-- > 0) {
    	asm("repeat #15998");
    	asm("nop");
	}
}

/* Set the clock speed and analog, input, and output statuses of 
 * the pic24 pins
 */
void pic24_setup(void){
	_RCDIV = 0;
	AD1PCFG = 0x7FFE;
	AD1PCFGbits.PCFG0 = 0; // set AN0 to analog
	AD1PCFGbits.PCFG1 = 0; // set AN1 to analog
	TRISBbits.TRISB12 = 0;   // Set RB12 as output
    TRISBbits.TRISB6 = 0; // Set RB6 as an output
	TRISAbits.TRISA0 = 1;  // Set RA0/AN0 to Input
	TRISAbits.TRISA1 = 1; // Set RA1/AN1 to Input
}

/* Gets a moving average of the ADC buffer associated with the Water Level Sensor
 * by decrementing and adding to a sum with a defined number of samples
 * all while checking for underflow
*/ 
unsigned int getAvgWaterLevel(){
	unsigned long sum = 0;
    	int index = buffer_index1;
    	for (int i = 0; i < NUMSAMPLES; i++) {
        	sum += adc_buffer1[index--]; // use decrementing index
        	if (index < 0) { // check for buffer underflow
            	index = BUFSIZE - 1;
        	}
    	}
    	unsigned int average = sum / NUMSAMPLES;
    	return average;
}

/* Gets a moving average of the ADC buffer associated with the Moisture Sensor 
 * by decrementing and adding to a sum with a defined number of samples
 * all while checking for underflow
*/ 
unsigned int getAvgMoisture(){
	unsigned long sum = 0;
    	int index = buffer_index2;
    	for (int i = 0; i < NUMSAMPLES; i++) {
        	sum += adc_buffer2[index--]; // use decrementing index
        	if (index < 0) { // check for buffer underflow
            	index = BUFSIZE - 1;
        	}
    	}
    	unsigned int average = sum / NUMSAMPLES;
    	return average;
}
/* Adds an integer parameter value to a buffer associated with the Water Level Sensor
 * attached to AN0 and checking for overflow
 */
void putVal1(unsigned int ADCvalue){
	adc_buffer1[buffer_index1++] = ADCvalue;
	if(buffer_index1 >= BUFSIZE){
    	buffer_index1 = 0;
	}
}

/* Adds an integer parameter value to a buffer associated with the Moisture Sensor
 * attached to AN1 and checking for overflow
 */
void putVal2(unsigned int ADCvalue){
	adc_buffer2[buffer_index2++] = ADCvalue;
	if(buffer_index2 >= BUFSIZE){
    	buffer_index2 = 0;
	}
}

/* Initializes 2 ADC buffers at a defined size to all 0 values
 */
void initBuffer(){
	int i;
	for(i=0; i < BUFSIZE; i++){
    	adc_buffer1[i] = INITWATERLEVEL;
    	adc_buffer2[i] = 0;
	}
}
/* Initializes ADC by assigning reference voltages, assigning timer3 and the sample time to the ADC,
 * and initializing AN0 as the ADC input
 */
void adc_init1(){
	AD1CON2bits.VCFG = 0b000; // VDD max (3.3V) and VSS min (0V)
    
	AD1CON3bits.ADCS = 1; // TAD >= 75 ns (Tcy = 62.5 ns)
	AD1CON1bits.SSRC = 2; // Sample on timer3 events
	AD1CON3bits.SAMC = 1; // At least 1 auto sample time bit
	AD1CON1bits.FORM = 0; // Unsigned int
	AD1CON1bits.ASAM = 1; // Reference Manual
	AD1CON2bits.SMPI = 0; // Reference Manual
	AD1CON1bits.ADON = 1; // Turn on ADC

    AD1CHSbits.CH0SA = 0;   // Select AN0 as the input 
    
	_AD1IF = 0;
	_AD1IE = 1; // Enable ADC interrupt
    
	TMR3 = 0;  // Set up timer3
	T3CON = 0;
	T3CONbits.TCKPS = 0b10;// 1:64
	PR3 = 15624/2;
	T3CONbits.TON = 1; // 32 times per second
}

/* For every sample time (Associated with timer3), a value is added to the corresponding 
 * ADC buffer to the current input pin (AN0/AN1) and then the ADC input is switched to the other pin
 */
void __attribute__((interrupt, auto_psv)) _ADC1Interrupt(void){
    if(AD1CHSbits.CH0SA == 0){
        putVal1(ADC1BUF0); // Call putVal() on adc_buffer1 with ADC1BUF0
        AD1CHSbits.CH0SA = 1; //Switch AD input to AN1
    }
    else if(AD1CHSbits.CH0SA == 1){
        putVal2(ADC1BUF0); // Call putVal() on adc_buffer2 with ADC1BUF0
        AD1CHSbits.CH0SA = 0; //Switch AD input to AN0
    }
	IFS0bits.AD1IF = 0; // Reset the ADC interrupt flag
}
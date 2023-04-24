/* 
 * File:   ADC_lib.h
 * Author: ian
 *
 * Created on April 24, 2023, 10:53 AM
 */

#ifndef ADC_LIB_H
#define	ADC_LIB_H

#ifdef	__cplusplus
extern "C" {
#endif
    
void delay_ms(unsigned int ms);
void pic24_setup(void);
void putVal1(unsigned int ADCvalue);
void putVal2(unsigned int ADCvalue);
void initBuffer();
void adc_init1();
void __attribute__((interrupt, auto_psv)) _ADC1Interrupt(void);



#ifdef	__cplusplus
}
#endif

#endif	/* ADC_LIB_H */


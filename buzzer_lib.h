/* 
 * File:   buzzer_lib.h
 * Author: kianm
 *
 * Created on April 24, 2023, 11:50 AM
 */

#ifndef BUZZER_LIB_H
#define	BUZZER_LIB_H

#ifdef	__cplusplus
extern "C" {
#endif
    
/* When called, RB6 is enabled, causing the buzzer to go of
 */
void buzzerEnable();


/* When called, buzzer stays on for 10 seconds 
 * or until the water threshold increases
 */
void buzzerDisable(volatile int* overflow);


/* Enables the RB12 pin
 */
void pumpEnable();


/* Disables the RB12 pin
 */
void pumpDisable();


#ifdef	__cplusplus
}
#endif

#endif	/* BUZZER_LIB_H */


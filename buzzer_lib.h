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

void buzzerEnable();
void buzzerDisable(volatile int overflow);


#ifdef	__cplusplus
}
#endif

#endif	/* BUZZER_LIB_H */


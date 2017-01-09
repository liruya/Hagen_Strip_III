/* 
 * File:   led.h
 * Author: liruya
 *
 * Created on November 1, 2016, 3:01 PM
 */

#ifndef LED_H
#define	LED_H

#ifdef	__cplusplus
extern "C" {
#endif

#include "comman.h"
    
#define FIND_FLASH_COUNT    3
#define PREVIEW_COUNT       3
#define MIN_LED_BRIGHT      10
#define MAX_LED_BRIGHT      1000
#define DELTA_LED_BRIGHT    200
#define LED_STEP_TOUCH      2
#define LED_DELTA_TOUCH     10    
#define STEP_NORMAL_BRIGHT  4
    
#define LED_STATUS_OFF      0
#define LED_STATUS_DAY      1
#define LED_STATUS_NIGHT    2
#define LED_STATUS_BLE      3
    
#define LED_RED_SetHigh()            do { LATBbits.LATB6 = 1; } while(0)
#define LED_RED_SetLow()             do { LATBbits.LATB6 = 0; } while(0)
#define LED_RED_Toggle()             do { LATBbits.LATB6 = ~LATBbits.LATB6; } while(0)
#define LED_BLUE_SetHigh()            do { LATBbits.LATB7 = 1; } while(0)
#define LED_BLUE_SetLow()             do { LATBbits.LATB7 = 0; } while(0)
#define LED_BLUE_Toggle()             do { LATBbits.LATB7 = ~LATBbits.LATB7; } while(0)
#define LED_GRN_SetHigh()            do { LATCbits.LATC7 = 1; } while(0)
#define LED_GRN_SetLow()             do { LATCbits.LATC7 = 0; } while(0)
#define LED_GRN_Toggle()             do { LATCbits.LATC7 = ~LATCbits.LATC7; } while(0)    
    
extern void updatePWM();
extern void updateLed();
extern void autoRun();
extern void turnOnLedRamp();
extern void turnOffLedRamp();
extern void turnOffLed();
extern void initLed();
extern void findDeviceFlash();
extern void preview();
extern void indicateLedOff();
extern void indicateLedDay();
extern void indicateLedNight();
extern void indicateLedBle();
extern void updateDayBright();
extern void updateDayRiseStatus();
extern void updateNightBright();
extern void updateNightRiseStatus();
    
#ifdef	__cplusplus
}
#endif

#endif	/* LED_H */


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

#define RGB_MAX_BRT         32
#define CW_MAX_BRT          32
    
extern void updatePara();
extern void updatePWM();
extern void updateLed();
extern void autoRun();
extern void initLed();
extern void findDeviceFlash();
extern void preview();
extern void updateDayBright();
extern void updateDayRiseStatus();
extern void updateNightBright();
extern void updateNightRiseStatus();
    
#ifdef	__cplusplus
}
#endif

#endif	/* LED_H */


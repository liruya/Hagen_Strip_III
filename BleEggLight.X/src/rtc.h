/* 
 * File:   rtc.h
 * Author: liruya
 *
 * Created on November 1, 2016, 1:15 PM
 */

#ifndef RTC_H
#define	RTC_H

#ifdef	__cplusplus
extern "C" {
#endif

#include "comman.h"
    
//current time YYMMdd D HHmmss
extern volatile DateTime_t gCurrentTime;

extern void initRTC();
extern void runRTC();


#ifdef	__cplusplus
}
#endif

#endif	/* RTC_H */


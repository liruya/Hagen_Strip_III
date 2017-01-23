/* 
 * File:   key.h
 * Author: liruya
 *
 * Created on November 10, 2016, 11:49 AM
 */

#ifndef KEY_H
#define	KEY_H

#ifdef	__cplusplus
extern "C" {
#endif

#include "comman.h"

#define KEY_PORT        PORTC
#define KEY_MASK        0x01        //RC0
#define KEY_NONE        0x00
#define KEY_PRESS       0x01
    
//#define KEY_PORT            PORTA
//#define KEY_MASK            0x02        //RA1
//#define KEY_NONE            0x00
//#define KEY_PRESS           0x02    
//#define KEY_LONG_MASK       0x80
//#define KEY_LONG_PRESS      (KEY_LONG_MASK | KEY_PRESS)    
#define KEY_CONT_MASK       0x40
#define KEY_CONT_PRESS      (KEY_CONT_MASK | KEY_PRESS)
#define KEY_RELEASE_MASK    0x20
#define KEY_RELEASE         (KEY_RELEASE_MASK | KEY_PRESS)
    
#define	RPT_COUNT		25                  //20ms*25=0.5s
#define LONG_COUNT      100                 //20ms*100=2s
#define	KEY_MINTIME		2					//按键持续时间最小值
#define	CONT_COUNT		1					//20ms
    
extern volatile unsigned char keyValue;

/**
 * scan for key pressed
 */
extern void keyScan();
extern void keyAction();

#ifdef	__cplusplus
}
#endif

#endif	/* KEY_H */


/* 
 * File:   ble.h
 * Author: Administrator
 *
 * Created on 2016?10?15?, ??3:04
 */

#ifndef BLE_H
#define	BLE_H

#ifdef	__cplusplus
extern "C"
{
#endif

#include "comman.h"
	
extern  volatile EusartRunPara_t eusartRunPara;	

extern	void initBLE();
extern	void DecodeCMD();
extern	void DecodeDATA();

#ifdef	__cplusplus
}
#endif

#endif	/* BLE_H */


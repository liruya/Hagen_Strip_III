/* 
 * File:   comman.h
 * Author: Administrator
 *
 * Created on 2016?10?15?, ??11:31
 */

#ifndef COMMAN_H
#define	COMMAN_H

#ifdef	__cplusplus
extern "C" {
#endif

#include <xc.h>

#define INTERRUPT_GlobalInterruptEnable() (INTCONbits.GIE = 1)
#define INTERRUPT_GlobalInterruptDisable() (INTCONbits.GIE = 0)
#define INTERRUPT_PeripheralInterruptEnable() (INTCONbits.PEIE = 1)
#define INTERRUPT_PeripheralInterruptDisable() (INTCONbits.PEIE = 0)
    
#define _XTAL_FREQ  16000000
    
//ble module pins
#define BLE_RST     LATB5
#define BLE_WKP		LATC1
#define BLE_INT		RA2
    
#define DEVICE_ID_VERSION   "3031313130313031"	
#define CRLF			"\r\n"
#define BLE_CMD_TEST	"AT\r\n"
#define BLE_CMD_SLAVE	"AT+SET=1\r\n"
#define BLE_CMD_DATA	"AT+MODE=DATA\r\n"
#define BLE_CMD_SET_ADV "AT+ADV_MFR_SPC=3031313130313031\r\n"       //id 0105 version 0101
#define BLE_CMD_GET_ADV "AT+ADV_MFR_SPC=?\r\n" 
#define BLE_CMD_ACK_OK	"AT+OK\r\n"	
	
#define	OVERTIME		30			//unit 4ms
#define	RESENDCOUNT		3
#define	RCVINTERVAL		15			//unit 4ms
	
//led pins
#define LED_RED		LATB7
#define LED_GRN		LATB6
#define PWM1		LATC5
#define PWM2		LATC4
#define PWM3		LATC3
#define PWM4		LATC6
	
#define CHANNEL_COUNT   4
#define CUSTOM_COUNT    4
    
//para defines
#define EEPROM_START_ADDR   0x7000          //eeprom start addr
#define ADDR_LED_PARA       0x10            //led para saved offset addr
#define PARA_SAVE_DELAY     3               //para saved delay if there is no para changed    

typedef	struct{
	unsigned fMode			: 1;	//0-cmd or 1-data
	unsigned fWaitForAck	: 1;	//wait for ack after cmd
	unsigned fAck			: 1;	//
	unsigned fResd			: 1;	//resend
	unsigned fComErr		: 1;	//communication error
	unsigned fRcving		: 1;
    unsigned fDev           : 1;
	unsigned fRsv			: 1;
    
    unsigned char	nOverTime;			
	unsigned char	nReSendCnt;			
	unsigned char	nCmd;				
	unsigned char	nRcvIdx;			
	unsigned char	nRcvInterval;		
	unsigned char	nRcvState;
}EusartRunPara_t;

typedef union{
    struct{
        unsigned char year;
        unsigned char month;
        unsigned char day;
        unsigned char wk;
        unsigned char hour;
        unsigned char minute;
        unsigned char second;
    }datetime;
    unsigned char array[7];
}DateTime_t;

typedef struct{
    unsigned char hour;
    unsigned char minute;
}Time_t;

typedef struct{
    Time_t start;
    Time_t end;
}TimeRamp_t;

typedef union{
    struct{
        unsigned fOn : 1;
        unsigned fRsv : 7;
        unsigned char fDyn;
        unsigned int nBrt[CHANNEL_COUNT];
        unsigned char nCustomBrt[4][CHANNEL_COUNT];
    }manualPara;
    unsigned char array[6*CHANNEL_COUNT+2];
}ManualPara_t;

typedef union{
    struct{
        TimeRamp_t sunrise;
        unsigned char daylight[CHANNEL_COUNT];
        TimeRamp_t sunset;
        unsigned char moonlight[CHANNEL_COUNT];
    }autoPara;
    unsigned char array[2*CHANNEL_COUNT+8];
}AutoPara_t;

typedef struct{
    unsigned int nDayBrt;
    unsigned int nNightBrt;
}TouchPara_t;

typedef struct{
    unsigned fAuto : 1;
    unsigned fSta : 2;
    unsigned fRsv : 5;
    ManualPara_t mPara;
    AutoPara_t aPara;
    TouchPara_t tPara;
}LedPara_t;

typedef struct{
    unsigned find : 1;
    unsigned fSec : 1;
    unsigned fSave : 1;
    unsigned fPrev : 1;
    unsigned fDr : 1;
    unsigned fNr : 1;
    unsigned fRsv : 2;
    unsigned char nFlashCount;
    unsigned char nPrevCount;
    unsigned char nSaveDelayCount;
    unsigned int nTargetBrt[CHANNEL_COUNT];
    unsigned int nCurrentBrt[CHANNEL_COUNT];
    unsigned char nCustomIndex;
}LedRunPara_t;

extern  volatile LedRunPara_t gLedRunPara;
extern  volatile LedPara_t    gLedPara;

extern  void SYSTEM_Initialize();
extern	unsigned char strCmp(const unsigned char *s1, const unsigned char *s2);

#ifdef	__cplusplus
}
#endif

#endif	/* COMMAN_H */


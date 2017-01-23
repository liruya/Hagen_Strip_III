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
#define BLE_RST     LATB4
#define BLE_WKP		LATC0
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
	
#define CHANNEL_COUNT   5
#define CUSTOM_COUNT    4
#define TIMER_COUNT     8
    
#define POWER_OFF           0x00
#define POWER_ON_RGB        0x01
#define POWER_ON_CW         0x02
#define POWER_ON_ALL        0x03
    
//para defines
#define EEPROM_START_ADDR   0x7000          //eeprom start addr
#define ADDR_MANUAL_PARA    0x10            //led para saved offset addr
#define ADDR_TIMER_PARA     0x20
#define PARA_SAVE_DELAY     3               //para saved delay if there is no para changed    

typedef	struct{
	unsigned fMode		: 1;	//0-cmd or 1-data
	unsigned fWaitForAck	: 1;	//wait for ack after cmd
	unsigned fAck		: 1;	//
	unsigned fResd		: 1;	//resend
	unsigned fComErr	: 1;	//communication error
	unsigned fRcving	: 1;
        unsigned fDev           : 1;
	unsigned fRsv		: 1;
    
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

typedef union
{
    struct
    {
        unsigned char fOn;
        unsigned char red;
        unsigned char green;
        unsigned char blue;
        unsigned char rgb_brt;
        unsigned char cw;
        unsigned char w_brt;
    }cw_rgb;
    unsigned char array[7];
}Scene_t;

typedef union
{
    struct
    {
        union
        {
            struct
            {            
                unsigned sun : 1;
                unsigned mon : 1;
                unsigned tue : 1;
                unsigned wed : 1;
                unsigned thu : 1;
                unsigned fri : 1;
                unsigned sat : 1;
                unsigned en : 1;
            }s_wk;
            unsigned char wk;
        }uWeek;
        Time_t time;
        Scene_t scene;
    }timer;
    unsigned char array[10];
}AutoPara_t;

typedef struct{
    Scene_t mPara;
    AutoPara_t aPara[TIMER_COUNT];
}LedPara_t;

typedef struct{
    unsigned fSec : 1;
    unsigned fMin : 1;
    unsigned fSave : 1;
    unsigned find : 1;
    unsigned fRsv : 4;
    unsigned char nSaveDelayCount;
    unsigned char nFlashCount;
    unsigned int nTargetBrt[CHANNEL_COUNT];
    unsigned int nCurrentBrt[CHANNEL_COUNT];
}LedRunPara_t;

extern  volatile LedRunPara_t gLedRunPara;
extern  volatile LedPara_t    gLedPara;

extern  void SYSTEM_Initialize();
extern	unsigned char strCmp(const unsigned char *s1, const unsigned char *s2);

#ifdef	__cplusplus
}
#endif

#endif	/* COMMAN_H */


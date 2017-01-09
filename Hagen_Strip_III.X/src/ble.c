#include "ble.h"
#include "rtc.h"
#include "eeprom.h"
#include "eusart.h"
#include "led.h"

#define	RCV_STATE_START	0
#define	RCV_STATE_DATA1	1
#define	RCV_STATE_DATA2	2

#define	CMD_NONE		0
#define	CMD_TEST		1
#define	CMD_SET			2
#define	CMD_MODE		3
#define	CMD_NAME		4
#define	CMD_DEVID		5
#define	CMD_TIME		6
#define	CMD_PSWD		7
#define CMD_SET_MFR     8
#define CMD_READ_MFR    9

#define FRM_HDR             0x68
#define CMD_DATA_AUTO       0x02
#define CMD_DATA_ONOFF      0x03
#define CMD_DATA_CTRL       0x04
#define CMD_DATA_READ       0x05
#define CMD_DATA_CUSTOM     0x06
#define CMD_DATA_CYCLE      0x07
#define CMD_DATA_INCREASE   0x08
#define CMD_DATA_DECREASE   0x09
#define CMD_DATA_DYN        0x0A
#define CMD_DATA_PREVIEW    0x0B
#define CMD_DATA_STOPPREV   0x0C
#define CMD_DATA_READTIME   0x0D
#define CMD_DATA_SYNCTIME   0x0E
#define CMD_DATA_FIND       0x0F

volatile EusartRunPara_t eusartRunPara;

unsigned char getCRC(unsigned char *pbuf, unsigned char len)
{
    unsigned char result = 0x00;
    for(unsigned char i = 0; i < len; i++)
    {
        result ^= *(pbuf+i);
    }
    return result;
}

void bleSendCMD(const unsigned char *pBuf)
{
	eusartSendBuffer(pBuf);
	eusartRunPara.fWaitForAck	= 1;
	eusartRunPara.nReSendCnt	= 0;
	while(!eusartRunPara.fAck)
	{
		if(eusartRunPara.nOverTime >= OVERTIME)
		{
			eusartRunPara.nOverTime	= 0;			
			eusartRunPara.nReSendCnt	++;
			if(eusartRunPara.nReSendCnt >= RESENDCOUNT)
			{
				eusartRunPara.nReSendCnt	= 0;
				eusartRunPara.fWaitForAck	= 0;
				eusartRunPara.fComErr	= 1;
				break;
			}
			eusartSendBuffer(pBuf);
		}
	}
	eusartRunPara.fAck	= 0;
}

void bleSendData(const unsigned char *frame, unsigned char len)
{
	BLE_WKP = 0;
	__delay_us(800);
	while(len--)
	{
		eusartSendByte(*frame++);
	}
	//wait for the frame transmission completed
	while(!TRMT);	
	BLE_WKP = 1;
}

void bleSendDeviceData()
{
    unsigned char i;
    unsigned char len;
    txBuf[0] = FRM_HDR;
    txBuf[1] = CMD_DATA_READ;
    txBuf[2] = gLedPara.fAuto;
    if (gLedPara.fAuto) 
    {
        len = sizeof(gLedPara.aPara.array);
        for (i = 0; i < len; i++) 
        {
            txBuf[3+i] = gLedPara.aPara.array[i];
        }
    } 
    else 
    {
        len = sizeof(gLedPara.mPara.array);
        for (i = 0; i < len; i++) 
        {
            txBuf[3+i] = gLedPara.mPara.array[i];
        }
    }
    txBuf[len+3] = getCRC(txBuf, len+3);
    bleSendData(txBuf, len+4);
}

void bleSendDeviceTime()
{
    unsigned char len = sizeof(gCurrentTime.array);
    txBuf[0] = FRM_HDR;
    txBuf[1] = CMD_DATA_READTIME;   
    for (unsigned char i = 0; i < len; i++) 
    {
        txBuf[2+i] = gCurrentTime.array[i];
    }
    txBuf[9] = getCRC(txBuf, len+2);
    bleSendData(txBuf, len+3);
}

void DecodeCMD()
{
	if(eusartRunPara.nCmd == CMD_NONE)
    {
        eusartRunPara.nRcvIdx = 0;
        return;
    }
	if(eusartRunPara.nRcvIdx <= 2)
    {
		return;
    }
	if(rxBuf[eusartRunPara.nRcvIdx-2] == '\r' && rxBuf[eusartRunPara.nRcvIdx-1] == '\n')
	{
		rxBuf[eusartRunPara.nRcvIdx] = '\0';
		eusartRunPara.nRcvIdx	= 0;
		switch(eusartRunPara.nRcvState)
		{
			case	RCV_STATE_START:			
				if(strCmp(rxBuf, "AT+OK\r\n") == 0)
				{			
					switch(eusartRunPara.nCmd)
					{
						case	CMD_TEST:
						case	CMD_MODE:
						case	CMD_NAME:
                        case    CMD_SET_MFR:
							eusartRunPara.fAck	= 1;
							break;
						case	CMD_SET:
						case	CMD_DEVID:
						case	CMD_TIME:
                        case    CMD_READ_MFR:
							eusartRunPara.nRcvState	= RCV_STATE_DATA1;
							break;
						default:
							break;
					}
				}
				break;
			case	RCV_STATE_DATA1:
				switch(eusartRunPara.nCmd)
				{
					case	CMD_SET:
						if(strCmp(rxBuf, "AT+OK\r\n") == 0)
						{
							eusartRunPara.fAck	= 1;
							eusartRunPara.nRcvState	= RCV_STATE_START;
						}	
						break;
					case	CMD_DEVID:
						eusartRunPara.fAck	= 1;
						eusartRunPara.nRcvState	= RCV_STATE_START;
						break;
					case	CMD_TIME:
						eusartRunPara.nRcvState	= RCV_STATE_DATA2;
						break;
                    case    CMD_READ_MFR:
                        eusartRunPara.fAck	= 1;
						eusartRunPara.nRcvState	= RCV_STATE_START;
                        if (strCmp(rxBuf, DEVICE_ID_VERSION) == 0) 
                        {
                            eusartRunPara.fDev = 1;
                        }
                        break;
					default:
						break;
				}
				break;
			case	RCV_STATE_DATA2:
				if(eusartRunPara.nCmd == CMD_TIME)
				{
					eusartRunPara.fAck	= 1;
					eusartRunPara.nRcvState	= RCV_STATE_START;
				}
				break;
			default:
				break;
		}
	}
}

void DecodeDATA()
{
//    unsigned char ack[] = "OK\r\n";
    if(gLedRunPara.find || rxBuf[0] != FRM_HDR)
    {
        return;
    }
    switch(rxBuf[1])
    {
        case CMD_DATA_AUTO:
            if(getCRC(rxBuf, 3) == rxBuf[3])
            {
                if(rxBuf[2] == 0x00)
                {
                    gLedPara.fAuto = 0;
                    if (gLedPara.mPara.manualPara.fOn) 
                    {
                        turnOnLedRamp();
                    }
                    else
                    {
                        turnOffLedRamp();
                    }
                }
                else if(rxBuf[2] == 0x01)
                {
                    gLedPara.fAuto = 1;
                }        
                gLedRunPara.fSave = 1;
                gLedRunPara.nSaveDelayCount = PARA_SAVE_DELAY;
                bleSendDeviceData();
            }
            break;
            
        case CMD_DATA_ONOFF:
            if( !gLedPara.fAuto )
            {
                if(getCRC(rxBuf, 3) == rxBuf[3])
                {
                    if(rxBuf[2] == 0x00)
                    {
                        gLedPara.mPara.manualPara.fOn = 0;
                        turnOffLedRamp();
                    }
                    else if(rxBuf[2] == 0x01)
                    {
                        gLedPara.mPara.manualPara.fOn = 1;
                        turnOnLedRamp();
                    }
                    gLedRunPara.fSave = 1;
                    gLedRunPara.nSaveDelayCount = PARA_SAVE_DELAY;
                    bleSendDeviceData();
                }
            }
            break;
            
        case CMD_DATA_CTRL:
            if(gLedPara.mPara.manualPara.fOn && (!gLedPara.fAuto) )
            {
                gLedPara.mPara.manualPara.fDyn = 0;
                unsigned char len = 2+2*CHANNEL_COUNT;
                if (getCRC(rxBuf, len) == rxBuf[len]) 
                {
                    for (unsigned char i = 0; i < CHANNEL_COUNT; i++) 
                    {
                        unsigned int val = (rxBuf[2+2*i]<<8)|rxBuf[3+2*i];
                        if (val <= MAX_LED_BRIGHT) 
                        {
                            gLedPara.mPara.manualPara.nBrt[i] = val;
                            gLedRunPara.nTargetBrt[i] = val;
                        }
                    }
                    gLedRunPara.fSave = 1;
                    gLedRunPara.nSaveDelayCount = PARA_SAVE_DELAY;
                }   
            }
            break;
            
        case CMD_DATA_READ:
            if(rxBuf[0] ^ rxBuf[1] == rxBuf[2])
            {
                bleSendDeviceData();
            }
            break;
           
        case CMD_DATA_CUSTOM:
            if (rxBuf[2] < CUSTOM_COUNT && getCRC(rxBuf, 3) == rxBuf[3])
            {
                unsigned char idx = rxBuf[2];
                for (unsigned char i = 0; i < CHANNEL_COUNT; i++) 
                {
                    gLedPara.mPara.manualPara.nCustomBrt[idx][i] = gLedRunPara.nCurrentBrt[i]/10;
                }
                gLedRunPara.fSave = 1;
                gLedRunPara.nSaveDelayCount = PARA_SAVE_DELAY;
                bleSendDeviceData();
            }
            break;    
            
        case CMD_DATA_CYCLE:
            if (getCRC(rxBuf, 2*CHANNEL_COUNT+10) == rxBuf[2*CHANNEL_COUNT+10]) 
            {
                unsigned char len = sizeof(gLedPara.aPara.array);
                for (unsigned char i = 0; i < len; i++) 
                {
                    gLedPara.aPara.array[i] = rxBuf[2+i];
                }
                gLedRunPara.fSave = 1;
                gLedRunPara.nSaveDelayCount = PARA_SAVE_DELAY;
                bleSendDeviceData();
            }
            break;
            
        case CMD_DATA_INCREASE:
            if (gLedPara.mPara.manualPara.fOn && getCRC(rxBuf, 4) == rxBuf[4] && rxBuf[2] < CHANNEL_COUNT) 
            {
                unsigned char idx = rxBuf[2];
                if (gLedPara.mPara.manualPara.nBrt[idx] + rxBuf[3] < MAX_LED_BRIGHT) 
                {
                    gLedPara.mPara.manualPara.nBrt[idx] += rxBuf[3];
                    gLedRunPara.nTargetBrt[idx] = gLedPara.mPara.manualPara.nBrt[idx];
                }
                else
                {
                    gLedPara.mPara.manualPara.nBrt[idx] = MAX_LED_BRIGHT;
                    gLedRunPara.nTargetBrt[idx] = MAX_LED_BRIGHT;
                }
                gLedRunPara.fSave = 1;
                gLedRunPara.nSaveDelayCount = PARA_SAVE_DELAY;
            }
            break;
            
        case CMD_DATA_DECREASE:
            if (gLedPara.mPara.manualPara.fOn && getCRC(rxBuf, 4) == rxBuf[4] && rxBuf[2] < CHANNEL_COUNT) 
            {
                unsigned char idx = rxBuf[2];
                if (gLedPara.mPara.manualPara.nBrt[idx] > rxBuf[3]) 
                {
                    gLedPara.mPara.manualPara.nBrt[idx] -= rxBuf[3];
                    gLedRunPara.nTargetBrt[idx] = gLedPara.mPara.manualPara.nBrt[idx];
                }
                else
                {
                    gLedPara.mPara.manualPara.nBrt[idx] = 0;
                    gLedRunPara.nTargetBrt[idx] = 0;
                }
                gLedRunPara.fSave = 1;
                gLedRunPara.nSaveDelayCount = PARA_SAVE_DELAY;
            }
            break;
            
        case CMD_DATA_PREVIEW:
            if (gLedPara.fAuto) 
            {
                unsigned char len = 2+2*CHANNEL_COUNT;
                if (getCRC(rxBuf, len) == rxBuf[len]) 
                {
                    for (unsigned char i = 0; i < CHANNEL_COUNT; i++) 
                    {
                        unsigned int val = (rxBuf[2+2*i]<<8)|rxBuf[3+2*i];
                        if (val <= MAX_LED_BRIGHT) 
                        {
                            gLedRunPara.nCurrentBrt[i] = val;
                        }
                    }
                    updatePWM();
                    gLedRunPara.fPrev = 1;
                    gLedRunPara.nPrevCount = PREVIEW_COUNT;
                }  
            }
            break;
            
        case CMD_DATA_STOPPREV:
            if(rxBuf[0] ^ rxBuf[1] == rxBuf[2])
            {
                gLedRunPara.fPrev = 0;
                gLedRunPara.nPrevCount = 0;
            }
            break;
            
        case CMD_DATA_READTIME:
            if (rxBuf[0] ^ rxBuf[1] == rxBuf[2]) 
            {
                bleSendDeviceTime();
            }
            break;
            
        case CMD_DATA_SYNCTIME:
            if (getCRC(rxBuf, 9) == rxBuf[9]) 
            {
                gCurrentTime.datetime.year = rxBuf[2];
                gCurrentTime.datetime.month = rxBuf[3];
                gCurrentTime.datetime.day = rxBuf[4];
                gCurrentTime.datetime.wk = rxBuf[5];
                gCurrentTime.datetime.hour = rxBuf[6];
                gCurrentTime.datetime.minute = rxBuf[7];
                gCurrentTime.datetime.second = rxBuf[8];
                gLedPara.fSta = LED_STATUS_BLE;
                indicateLedBle();
                bleSendDeviceData();
            }
            break;
            
        case CMD_DATA_FIND:
            if (rxBuf[0] ^ rxBuf[1] == rxBuf[2]) 
            {
                gLedRunPara.find = 1;
                gLedRunPara.nFlashCount = FIND_FLASH_COUNT;
            }
            break;
           
        default:
            break;
    }
}

void initBLE()
{
	__delay_ms(5);
	BLE_RST = 1;								//release from reset
	__delay_ms(200);							//delay 200ms after power on
	eusartRunPara.nCmd	= CMD_TEST;
	bleSendCMD(BLE_CMD_TEST);
    
    eusartRunPara.nCmd	= CMD_READ_MFR;
	bleSendCMD(BLE_CMD_GET_ADV);
    
    if (!eusartRunPara.fDev) 
    {
        eusartRunPara.nCmd	= CMD_SET_MFR;
        bleSendCMD(BLE_CMD_SET_ADV);
    }
   
	eusartRunPara.nCmd	= CMD_SET;
	bleSendCMD(BLE_CMD_SLAVE);
	
	eusartRunPara.nCmd	= CMD_MODE;
	bleSendCMD(BLE_CMD_DATA);
	
	if(eusartRunPara.fComErr == 0)
		eusartRunPara.fMode	= 1;
}

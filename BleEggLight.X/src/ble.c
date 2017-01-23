#include "ble.h"
#include "rtc.h"
#include "eeprom.h"
#include "eusart.h"
#include "led.h"

#define	RCV_STATE_START	0
#define	RCV_STATE_DATA1	1
#define	RCV_STATE_DATA2	2

#define	CMD_NONE            0
#define	CMD_TEST            1
#define	CMD_SET             2
#define	CMD_MODE            3
#define	CMD_NAME            4
#define	CMD_DEVID           5
#define	CMD_TIME            6
#define	CMD_PSWD            7
#define CMD_SET_MFR         8
#define CMD_READ_MFR        9

#define FRM_HDR             0x68
#define CMD_DATA_ONOFF      0x03
#define CMD_DATA_CTRL       0x04
#define CMD_DATA_READ       0x05
#define CMD_DATA_TIMER      0x06
#define CMD_DATA_SET_TIMER  0x07
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

void bleSendManualData()
{
    unsigned char i;
    txBuf[0] = FRM_HDR;
    txBuf[1] = CMD_DATA_READ;
    for (i = 0; i < 7; i++) 
    {
        txBuf[2+i] = gLedPara.mPara.array[i];
    }
    txBuf[9] = getCRC(txBuf, 9);
    bleSendData(txBuf, 10);
}

unsigned char isValidTimer( AutoPara_t para )
{
    if (para.timer.time.hour > 23) 
    {
        return 0;
    }
    if (para.timer.time.minute > 59) 
    {
        return 0;
    }
    if (para.timer.scene.cw_rgb.fOn > POWER_ON_ALL) 
    {
        return 0;
    }
    if (para.timer.scene.cw_rgb.red > 32) 
    {
        return 0;
    }
    if (para.timer.scene.cw_rgb.green > 32) 
    {
        return 0;
    }
    if (para.timer.scene.cw_rgb.blue > 32) 
    {
        return 0;
    }
    if (para.timer.scene.cw_rgb.rgb_brt > 32) 
    {
        return 0;
    }
    if (para.timer.scene.cw_rgb.cw > 32) 
    {
        return 0;
    }
    if (para.timer.scene.cw_rgb.w_brt > 32) 
    {
        return 0;
    }
    return 1;
}

void bleSendTimerData()
{
    unsigned char i, j;
    unsigned char len = 0;
    unsigned char len;
    txBuf[0] = FRM_HDR;
    txBuf[1] = CMD_DATA_TIMER;
    for (i = 0; i < TIMER_COUNT; i++) 
    {
        if (isValidTimer(gLedPara.aPara[i]))
        {
            for (j = 0; j < 10; j++)
            {
                txBuf[3+10*len+j] = gLedPara.aPara[i].array[j];
            }
            len++;
        }   
        else
        {
            break;
        }
    }
    txBuf[2] = len;
    txBuf[3+len*10] = getCRC(txBuf, 3+len*10);
    bleSendData(txBuf, 4+len*10);
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
    unsigned char len;
    if(rxBuf[0] != FRM_HDR)
    {
        return;
    }
    switch(rxBuf[1])
    {       
        case CMD_DATA_ONOFF:
            if(getCRC(rxBuf, 3) == rxBuf[3])
            {
                if(rxBuf[2] <= POWER_ON_ALL)
                {
                    gLedPara.mPara.cw_rgb.fOn = rxBuf[2];
                    gLedRunPara.fSave = 1;
                    gLedRunPara.nSaveDelayCount = PARA_SAVE_DELAY;
                    bleSendManualData();
                    updatePara();
                }                
            }
            break;
            
        case CMD_DATA_CTRL:
            if(rxBuf[2] == POWER_ON_RGB )
            {
                len = 7;
                if (getCRC(rxBuf, len) == rxBuf[len]) 
                {
                    gLedPara.mPara.cw_rgb.fOn = POWER_ON_RGB;
                    gLedPara.mPara.cw_rgb.red = rxBuf[3];
                    gLedPara.mPara.cw_rgb.green = rxBuf[4];
                    gLedPara.mPara.cw_rgb.blue = rxBuf[5];
                    gLedPara.mPara.cw_rgb.rgb_brt = rxBuf[6];
                    gLedRunPara.fSave = 1;
                    gLedRunPara.nSaveDelayCount = PARA_SAVE_DELAY;
                    updatePara();
                }   
            }
            else if(rxBuf[2] == POWER_ON_CW )
            {
                len = 5;
                if (getCRC(rxBuf, len) == rxBuf[len]) 
                {
                    gLedPara.mPara.cw_rgb.fOn = POWER_ON_CW;
                    gLedPara.mPara.cw_rgb.cw = rxBuf[3];
                    gLedPara.mPara.cw_rgb.w_brt = rxBuf[4];
                    gLedRunPara.fSave = 1;
                    gLedRunPara.nSaveDelayCount = PARA_SAVE_DELAY;
                    updatePara();
                }   
            }
//            else if(rxBuf[2] == POWER_ON_ALL )
//            {
//                len = 9;
//                if (getCRC(rxBuf, len) == rxBuf[len]) 
//                {
//                    gLedPara.mPara.cw_rgb.fOn = POWER_ON_ALL;
//                    gLedPara.mPara.cw_rgb.red = rxBuf[3];
//                    gLedPara.mPara.cw_rgb.green = rxBuf[4];
//                    gLedPara.mPara.cw_rgb.blue = rxBuf[5];
//                    gLedPara.mPara.cw_rgb.rgb_brt = rxBuf[6];
//                    gLedPara.mPara.cw_rgb.cw = rxBuf[7];
//                    gLedPara.mPara.cw_rgb.w_brt = rxBuf[8];
//                    gLedRunPara.fSave = 1;
//                    gLedRunPara.nSaveDelayCount = PARA_SAVE_DELAY;
//                    updatePara();
//                }   
//            }
            break;
            
        case CMD_DATA_READ:
            if(rxBuf[0] ^ rxBuf[1] == rxBuf[2])
            {
                bleSendManualData();
            }
            break;  
            
        case CMD_DATA_TIMER:
            if(rxBuf[0] ^ rxBuf[1] == rxBuf[2])
            {
                bleSendTimerData();
            }
            break;
            
        case CMD_DATA_SET_TIMER:
            len = rxBuf[2];
            if (len > 0 && getCRC(rxBuf, 10*len+3) == rxBuf[10*len+3]) 
            {
                for (unsigned char i = 0; i < len; i++) 
                {
                    for (unsigned char j = 0; j < 10; j++) 
                    {
                        gLedPara.aPara[i].array[j] = rxBuf[3+i*10+j];
                    }
                }
                gLedRunPara.fSave = 1;
                gLedRunPara.nSaveDelayCount = PARA_SAVE_DELAY;
                bleSendTimerData();
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
                bleSendManualData();
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

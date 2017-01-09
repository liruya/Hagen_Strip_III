#include "key.h"
#include "led.h"
#include "eeprom.h"

volatile unsigned char keyValue = KEY_NONE;

/**
 * 
 * @return 
 */
static unsigned char ReadKey()
{
	static unsigned char	Trg;		
	static unsigned char	Cont;		
	unsigned char ReadData = (KEY_PORT ^ 0xFF) & KEY_MASK;
    Trg = ReadData & (ReadData ^ Cont);
    Cont = ReadData;
	return (Trg ^ Cont);
}

void keyScan()
{	
	static unsigned char lastKey	= KEY_NONE;					
	static unsigned char rptCount	= 0;	
	
	unsigned char keyNum	= ReadKey();					
	if(keyNum != KEY_NONE)									
	{
		if(keyNum == lastKey)								
		{		
            rptCount++;
			if(rptCount >= RPT_COUNT + CONT_COUNT)			
			{
                rptCount = RPT_COUNT;
				keyValue = KEY_CONT_MASK | keyNum;
			}
		}
		else
		{
			rptCount = 0;	
		}
	}
	else									
	{
		if(lastKey != KEY_NONE)				
		{
            if (rptCount >= RPT_COUNT) 
            {
                keyValue = KEY_RELEASE_MASK | lastKey;
            }
            else if( rptCount > KEY_MINTIME )	
			{
				keyValue = lastKey;
			}
		}
		rptCount = 0;	
	}
	lastKey	= keyNum;
}

void keyAction()
{
    unsigned char sta;
    if (keyValue == KEY_PRESS) 
    {
        gLedPara.fSta++;
        sta = gLedPara.fSta;
        switch(sta)
        {
            case LED_STATUS_OFF:
                for (unsigned char i = 0; i < CHANNEL_COUNT; i++) 
                {
                    gLedPara.mPara.manualPara.nBrt[i] = 0;
                    gLedRunPara.nCurrentBrt[i] = 0;
                }
                updatePWM();
//                turnOffLed();
                indicateLedOff();
                break;
                
            case LED_STATUS_DAY:
                updateDayBright();
//                for (unsigned char i = 0; i < CHANNEL_COUNT; i++) 
//                {
//                    gLedRunPara.nCurrentBrt[i] = gLedPara.mPara.manualPara.nBrt[i];
//                }
//                turnOnLedRamp();
                indicateLedDay();
                updateDayRiseStatus();
                break;
                
            case LED_STATUS_NIGHT:
                updateNightBright();
//                for (unsigned char i = 0; i < CHANNEL_COUNT; i++) 
//                {
//                    gLedRunPara.nCurrentBrt[i] = gLedPara.mPara.manualPara.nBrt[i];
//                }
//                turnOnLedRamp();
                indicateLedNight();
                updateNightRiseStatus();
                break;
                
            case LED_STATUS_BLE:
//                turnOnLed();
                indicateLedBle();
                gLedPara.fAuto = 1;
                break;
        }
    }
    else if (keyValue == KEY_RELEASE) 
    {
        sta = gLedPara.fSta;
        switch(sta)
        {
            case LED_STATUS_DAY:
                updateDayRiseStatus();
                break;
                
            case LED_STATUS_NIGHT:
                updateNightRiseStatus();
                break;
                
            default:
                break;
        }
    }
    else if ( keyValue == KEY_CONT_PRESS )
    {       
        sta = gLedPara.fSta;
        switch(sta)
        {
            case LED_STATUS_OFF:
                gLedPara.tPara.nDayBrt = MIN_LED_BRIGHT;
                gLedPara.fSta++;
                gLedRunPara.fDr = 1;
//                turnOnLedRamp();
                updateDayBright();
                indicateLedDay();
                break;
                
            case LED_STATUS_DAY:
                if (gLedRunPara.fDr) 
                {
                    if (gLedPara.tPara.nDayBrt + LED_DELTA_TOUCH < MAX_LED_BRIGHT) 
                    {
                        gLedPara.tPara.nDayBrt += LED_DELTA_TOUCH;
                    }
                    else
                    {
                        gLedPara.tPara.nDayBrt = MAX_LED_BRIGHT;
                    }
                }
                else
                {
                    if (gLedPara.tPara.nDayBrt > MIN_LED_BRIGHT + LED_DELTA_TOUCH) 
                    {
                        gLedPara.tPara.nDayBrt -= LED_DELTA_TOUCH;
                    }
                    else
                    {
                        gLedPara.tPara.nDayBrt = MIN_LED_BRIGHT;
                    }
                }
                updateDayBright();
                break;
                
            case LED_STATUS_NIGHT:
                if (gLedRunPara.fNr) 
                {
                    if (gLedPara.tPara.nNightBrt + LED_DELTA_TOUCH < MAX_LED_BRIGHT) 
                    {
                        gLedPara.tPara.nNightBrt += LED_DELTA_TOUCH;
                    }
                    else
                    {
                        gLedPara.tPara.nNightBrt = MAX_LED_BRIGHT;
                    }
                }
                else
                {
                    if (gLedPara.tPara.nNightBrt > MIN_LED_BRIGHT + LED_DELTA_TOUCH) 
                    {
                        gLedPara.tPara.nNightBrt -= LED_DELTA_TOUCH;
                    }
                    else
                    {
                        gLedPara.tPara.nNightBrt = MIN_LED_BRIGHT;
                    }
                }
                updateNightBright();
                break;
        }
    }
    gLedRunPara.fSave = 1;
    gLedRunPara.nSaveDelayCount = PARA_SAVE_DELAY;
}
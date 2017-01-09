#include "led.h"
#include "eeprom.h"
#include "rtc.h"
#include "key.h"
#include "pwm.h"

void updatePWM()
{    
    PWM3_LoadDutyValue( gLedRunPara.nCurrentBrt[0] );           
    PWM2_LoadDutyValue( gLedRunPara.nCurrentBrt[1] );
    PWM4_LoadDutyValue( gLedRunPara.nCurrentBrt[2] ); 
    PWM1_LoadDutyValue( gLedRunPara.nCurrentBrt[3] );
}

void updateLed()
{
    for (unsigned char i = 0; i < CHANNEL_COUNT; i++) 
    {
        if (gLedRunPara.nCurrentBrt[i] == gLedRunPara.nTargetBrt[i]) 
        {
            continue;
        }
        else
        {
            if (gLedRunPara.nCurrentBrt[i] + STEP_NORMAL_BRIGHT < gLedRunPara.nTargetBrt[i]) 
            {
                gLedRunPara.nCurrentBrt[i] += STEP_NORMAL_BRIGHT;
            }
            else if(gLedRunPara.nTargetBrt[i] + STEP_NORMAL_BRIGHT < gLedRunPara.nCurrentBrt[i]) 
            {
                gLedRunPara.nCurrentBrt[i] -= STEP_NORMAL_BRIGHT;
            }
            else
            {
                gLedRunPara.nCurrentBrt[i] = gLedRunPara.nTargetBrt[i];
            }
        }
    }
    updatePWM();
}

void autoRun()
{
    unsigned char i, j;
    unsigned int ct = gCurrentTime.datetime.hour*60u + gCurrentTime.datetime.minute;
    unsigned char sec = gCurrentTime.datetime.second;
    unsigned int tr[4];
    tr[0] = gLedPara.aPara.autoPara.sunrise.start.hour*60u + gLedPara.aPara.autoPara.sunrise.start.minute;
    tr[1] = gLedPara.aPara.autoPara.sunrise.end.hour*60u + gLedPara.aPara.autoPara.sunrise.end.minute;
    tr[2] = gLedPara.aPara.autoPara.sunset.start.hour*60u + gLedPara.aPara.autoPara.sunset.start.minute;
    tr[3] = gLedPara.aPara.autoPara.sunset.end.hour*60u + gLedPara.aPara.autoPara.sunset.end.minute;
    unsigned char val[4][CHANNEL_COUNT];
    for (i = 0; i < CHANNEL_COUNT; i++) 
    {
        val[0][i] = gLedPara.aPara.autoPara.moonlight[i];
        val[1][i] = gLedPara.aPara.autoPara.daylight[i];
        val[2][i] = gLedPara.aPara.autoPara.daylight[i];
        val[3][i] = gLedPara.aPara.autoPara.moonlight[i];
    }
    for (i = 0; i < 4; i++) 
    {
        j = (i+1)%4;
        unsigned int st = tr[i];
        unsigned int et = tr[j];
        unsigned int duration;
        unsigned long dt;
        unsigned char dbrt;
        if (et >= st) 
        {
            if (ct >= st && ct < et) 
            {
                duration = et - st;
                dt = (ct - st) * 60u + sec;
            }
            else
            {
                continue;
            }
        } 
        else 
        {
            if (ct >= st || ct < et) 
            {
                duration = 1440u - st + et;
                if (ct >= st) 
                {
                    dt = (ct - st) * 60u + sec;
                }
                else
                {
                    dt = (1440u - st + ct) * 60u + sec;
                }
            }
            else
            {
                continue;
            }
        }
        for (unsigned char k = 0; k < CHANNEL_COUNT; k++) 
        {
            if (val[j][k] >= val[i][k]) 
            {
                dbrt = val[j][k] - val[i][k];
                gLedRunPara.nCurrentBrt[k] = val[i][k] * 10u + dbrt * dt / (duration * 6u);
            } 
            else 
            {
                dbrt = val[i][k] - val[j][k];
                gLedRunPara.nCurrentBrt[k] = val[i][k] * 10u - dbrt * dt / (duration * 6u);
            }   
        }
    }
    updatePWM();
}

/**
 * turn on led
 */
void turnOnLedRamp()
{
    for (unsigned char i = 0; i < CHANNEL_COUNT; i++) 
    {
        gLedRunPara.nTargetBrt[i] = gLedPara.mPara.manualPara.nBrt[i];
    }
}

/**
 * turn off led ramp
 */
void turnOffLedRamp()
{
    for (unsigned char i = 0; i < CHANNEL_COUNT; i++) 
    {
        gLedRunPara.nTargetBrt[i] = 0;
    }
}

void turnMaxLed()
{
    PWM1_LoadDutyValue( MAX_LED_BRIGHT );           
    PWM2_LoadDutyValue( MAX_LED_BRIGHT );
    PWM3_LoadDutyValue( MAX_LED_BRIGHT ); 
    PWM4_LoadDutyValue( MAX_LED_BRIGHT );
}

/**
 * turn off led
 */
void turnOffLed()
{
    PWM1_LoadDutyValue( 0 );           
    PWM2_LoadDutyValue( 0 );
    PWM3_LoadDutyValue( 0 ); 
    PWM4_LoadDutyValue( 0 );
}

void initLedPara()
{
    gLedPara = readLedPara( ADDR_LED_PARA );
    gLedPara.fRsv = 0;
    gLedPara.mPara.manualPara.fRsv = 0;
    for (unsigned char i = 0; i < CHANNEL_COUNT; i++) 
    {
        //manual light check
        if (gLedPara.mPara.manualPara.nBrt[i] > MAX_LED_BRIGHT) 
        {
            gLedPara.mPara.manualPara.nBrt[i] = MAX_LED_BRIGHT;
        }
        //custom light check
        for (unsigned char j = 0; j < 4; j++) 
        {
            if (gLedPara.mPara.manualPara.nCustomBrt[j][i] > 100) 
            {
                gLedPara.mPara.manualPara.nCustomBrt[j][i] = 100;
            }
        }
        //day light check
        if (gLedPara.aPara.autoPara.daylight[i] > 100) 
        {
            gLedPara.aPara.autoPara.daylight[i] = 100;
        }
        //night light check
        if (gLedPara.aPara.autoPara.moonlight[i] > 100) 
        {
            gLedPara.aPara.autoPara.moonlight[i] = 100;
        }
    }
    
    //sunrise start time check
    if (gLedPara.aPara.autoPara.sunrise.start.hour > 23) 
    {
        gLedPara.aPara.autoPara.sunrise.start.hour = 0;
    }
    if (gLedPara.aPara.autoPara.sunrise.start.minute > 59) 
    {
        gLedPara.aPara.autoPara.sunrise.start.minute = 0;
    }
    //sunrise end time check
    if (gLedPara.aPara.autoPara.sunrise.end.hour > 23) 
    {
        gLedPara.aPara.autoPara.sunrise.end.hour = 0;
    }
    if (gLedPara.aPara.autoPara.sunrise.end.minute > 59) 
    {
        gLedPara.aPara.autoPara.sunrise.end.minute = 0;
    }
    //sunset start time check
    if (gLedPara.aPara.autoPara.sunset.start.hour > 23) 
    {
        gLedPara.aPara.autoPara.sunset.start.hour = 0;
    }
    if (gLedPara.aPara.autoPara.sunset.start.minute > 59) 
    {
        gLedPara.aPara.autoPara.sunset.start.minute = 0;
    }
    //sunset end time check
    if (gLedPara.aPara.autoPara.sunset.end.hour > 23) 
    {
        gLedPara.aPara.autoPara.sunset.end.hour = 0;
    }
    if (gLedPara.aPara.autoPara.sunset.end.minute > 59) 
    {
        gLedPara.aPara.autoPara.sunset.end.minute = 0;
    }
    
    if (gLedPara.tPara.nDayBrt > MAX_LED_BRIGHT) 
    {
        gLedPara.tPara.nDayBrt = MAX_LED_BRIGHT;
    }
    else if (gLedPara.tPara.nDayBrt < MIN_LED_BRIGHT) 
    {
        gLedPara.tPara.nDayBrt = MIN_LED_BRIGHT;
    }
    updateDayRiseStatus();
    if (gLedPara.tPara.nNightBrt > MAX_LED_BRIGHT) 
    {
        gLedPara.tPara.nNightBrt = MAX_LED_BRIGHT;
    } 
    else if (gLedPara.tPara.nNightBrt < MIN_LED_BRIGHT) 
    {
        gLedPara.tPara.nNightBrt = MIN_LED_BRIGHT;
    }
    updateNightRiseStatus();
}

void initLedStatus()
{
    unsigned char sta = gLedPara.fSta;
    switch(sta)
    {
        case LED_STATUS_OFF:
            turnOffLedRamp();
            indicateLedOff();
            break;
            
        case LED_STATUS_DAY:
            updateDayBright();
            indicateLedDay();
            turnOnLedRamp();
            break;
            
        case LED_STATUS_NIGHT:
            updateNightBright();
            indicateLedNight();
            turnOnLedRamp();
            break;
            
        case LED_STATUS_BLE:
            indicateLedBle();
            if (!gLedPara.fAuto) 
            {
                for (unsigned char i = 0; i < CHANNEL_COUNT; i++) 
                {
                    gLedRunPara.nTargetBrt[i] = gLedPara.mPara.manualPara.nBrt[i];
                }
            }
            break;
        
        default:
            break;
    }
}

void initLed()
{
    initLedPara();
    initLedStatus();
}

/**
 * when find device led flashes
 */
void findDeviceFlash()
{
    static unsigned char tog = 0;
    if (tog) 
    {
        tog = 0;
        turnOffLed();
        if (gLedRunPara.nFlashCount > 0) 
        {
            gLedRunPara.nFlashCount --;
        } 
        if (gLedRunPara.nFlashCount == 0) 
        {
            gLedRunPara.find = 0;
            //recovery status before find device
            initLedStatus();
        }
    }
    else
    {
        tog = 1;
        turnMaxLed();
    }
}

void preview()
{
    if (gLedRunPara.nPrevCount > 0) 
    {
        gLedRunPara.nPrevCount--;
    }
    if (gLedRunPara.nPrevCount == 0) 
    {
        gLedRunPara.fPrev = 0;
    } 
}

//led turn off indicate - red led
void indicateLedOff()
{
    LED_RED_SetLow();
    LED_GRN_SetHigh();
    LED_BLUE_SetHigh();
}

//day light led indicate - white led
void indicateLedDay()
{
    LED_RED_SetLow();
    LED_GRN_SetLow();
    LED_BLUE_SetLow();
}

//night light indicate - blue led
void indicateLedNight()
{
    LED_RED_SetHigh();
    LED_GRN_SetHigh();
    LED_BLUE_SetLow();
}

//ble mode indicate - green led
void indicateLedBle()
{
    LED_RED_SetHigh();
    LED_GRN_SetLow();
    LED_BLUE_SetHigh();
}

void updateDayBright()
{
    for (unsigned char i = 0; i < CHANNEL_COUNT; i++)
    {
        gLedPara.mPara.manualPara.nBrt[i] = gLedPara.tPara.nDayBrt;
        gLedRunPara.nCurrentBrt[i] = gLedPara.tPara.nDayBrt;
    }
    updatePWM();
}

void updateDayRiseStatus()
{
    if (gLedPara.tPara.nDayBrt > MAX_LED_BRIGHT - DELTA_LED_BRIGHT) 
    {
        gLedRunPara.fDr = 0;
    }
    else if (gLedPara.tPara.nDayBrt < DELTA_LED_BRIGHT) 
    {
        gLedRunPara.fDr = 1;
    }
}

void updateNightBright()
{
    for (unsigned char i = 0; i < CHANNEL_COUNT; i++)
    {
        if (i == 2) 
        {
            gLedPara.mPara.manualPara.nBrt[i] = gLedPara.tPara.nNightBrt;
            gLedRunPara.nCurrentBrt[i] = gLedPara.tPara.nNightBrt;
        }
        else 
        {
            gLedPara.mPara.manualPara.nBrt[i] = 0;
            gLedRunPara.nCurrentBrt[i] = 0;
        }      
    }
    updatePWM();
}

void updateNightRiseStatus()
{
    if (gLedPara.tPara.nNightBrt > MAX_LED_BRIGHT - DELTA_LED_BRIGHT) 
    {
        gLedRunPara.fNr = 0;
    }
    else if (gLedPara.tPara.nNightBrt < DELTA_LED_BRIGHT) 
    {
        gLedRunPara.fNr = 1;
    }
}

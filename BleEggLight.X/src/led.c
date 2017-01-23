#include "led.h"
#include "eeprom.h"
#include "rtc.h"
#include "key.h"
#include "pwm.h"
#include "ble.h"

void updatePara()
{
    if (gLedPara.mPara.cw_rgb.fOn & POWER_ON_RGB) 
    {
        gLedRunPara.nTargetBrt[0] = gLedPara.mPara.cw_rgb.red * gLedPara.mPara.cw_rgb.rgb_brt;
        gLedRunPara.nTargetBrt[1] = gLedPara.mPara.cw_rgb.green * gLedPara.mPara.cw_rgb.rgb_brt;
        gLedRunPara.nTargetBrt[2] = gLedPara.mPara.cw_rgb.blue * gLedPara.mPara.cw_rgb.rgb_brt;
    }
    else
    {
        gLedRunPara.nTargetBrt[0] = 0;
        gLedRunPara.nTargetBrt[1] = 0;
        gLedRunPara.nTargetBrt[2] = 0;
    }
    if (gLedPara.mPara.cw_rgb.fOn & POWER_ON_CW) 
    {
        gLedRunPara.nTargetBrt[3] = (CW_MAX_BRT - gLedPara.mPara.cw_rgb.cw) * gLedPara.mPara.cw_rgb.w_brt;
        gLedRunPara.nTargetBrt[4] = gLedPara.mPara.cw_rgb.cw * gLedPara.mPara.cw_rgb.w_brt;
    }
    else
    {
        gLedRunPara.nTargetBrt[3] = 0;
        gLedRunPara.nTargetBrt[4] = 0;
    }
    for (unsigned char i = 0; i < CHANNEL_COUNT; i++) 
    {
        if (gLedRunPara.nTargetBrt[i] > MAX_LED_BRIGHT) 
        {
            gLedRunPara.nTargetBrt[i] = MAX_LED_BRIGHT;
        }
    }
}

void updatePWM()
{    
    PWM1_LoadDutyValue( gLedRunPara.nCurrentBrt[0] );           
    PWM2_LoadDutyValue( gLedRunPara.nCurrentBrt[1] );
    PWM3_LoadDutyValue( gLedRunPara.nCurrentBrt[2] ); 
    PWM4_LoadDutyValue( gLedRunPara.nCurrentBrt[3] );
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
    unsigned char i, hour, min;
    for (i = 0; i < TIMER_COUNT; i++) 
    {
        if (gLedPara.aPara[i].timer.uWeek.s_wk.en && isValidTimer(gLedPara.aPara[i]) ) 
        {
            hour = gLedPara.aPara[i].timer.time.hour;
            min = gLedPara.aPara[i].timer.time.minute;
            if (gCurrentTime.datetime.hour == hour && gCurrentTime.datetime.minute == min) 
            {
                gLedPara.mPara = gLedPara.aPara[i].timer.scene;
                if (gLedPara.aPara[i].timer.uWeek.wk == 0x80) 
                {
                    gLedPara.aPara[i].timer.uWeek.wk = 0x00;
                }
                gLedRunPara.fSave = 1;
                gLedRunPara.nSaveDelayCount = PARA_SAVE_DELAY;
                updatePara();
                return;
            }
        }       
    }
}

void turnMaxLed()
{
    PWM1_LoadDutyValue( MAX_LED_BRIGHT );           
    PWM2_LoadDutyValue( MAX_LED_BRIGHT );
    PWM3_LoadDutyValue( MAX_LED_BRIGHT ); 
    PWM4_LoadDutyValue( MAX_LED_BRIGHT );
    PWM5_LoadDutyValue( MAX_LED_BRIGHT );
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
    PWM5_LoadDutyValue( 0 );
}

void initLedPara()
{
    readLedPara();
    if (gLedPara.mPara.cw_rgb.fOn > POWER_ON_ALL) 
    {
        gLedPara.mPara.cw_rgb.fOn = POWER_OFF;
    }
    for (unsigned char i = 1; i < 7; i++) 
    {
        //manual light check
        if (gLedPara.mPara.array[i] > 32) 
        {
            gLedPara.mPara.array[i] = 32;
        }
    }
}

void initLed()
{
    initLedPara();
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
        }
    }
    else
    {
        tog = 1;
        turnMaxLed();
    }
}
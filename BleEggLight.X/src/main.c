/**
  Generated Main Source File

  Company:
    Microchip Technology Inc.

  File Name:
    main.c

  Summary:
    This is the main file generated using MPLAB(c) Code Configurator

  Description:
    This header file provides implementations for driver APIs for all modules selected in the GUI.
    Generation Information :
        Product Revision  :  MPLAB(c) Code Configurator - 3.16
        Device            :  PIC16F18345
        Driver Version    :  2.00
    The generated drivers are tested against the following:
        Compiler          :  XC8 1.35
        MPLAB             :  MPLAB X 3.20
*/

/*
    (c) 2016 Microchip Technology Inc. and its subsidiaries. You may use this
    software and any derivatives exclusively with Microchip products.

    THIS SOFTWARE IS SUPPLIED BY MICROCHIP "AS IS". NO WARRANTIES, WHETHER
    EXPRESS, IMPLIED OR STATUTORY, APPLY TO THIS SOFTWARE, INCLUDING ANY IMPLIED
    WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY, AND FITNESS FOR A
    PARTICULAR PURPOSE, OR ITS INTERACTION WITH MICROCHIP PRODUCTS, COMBINATION
    WITH ANY OTHER PRODUCTS, OR USE IN ANY APPLICATION.

    IN NO EVENT WILL MICROCHIP BE LIABLE FOR ANY INDIRECT, SPECIAL, PUNITIVE,
    INCIDENTAL OR CONSEQUENTIAL LOSS, DAMAGE, COST OR EXPENSE OF ANY KIND
    WHATSOEVER RELATED TO THE SOFTWARE, HOWEVER CAUSED, EVEN IF MICROCHIP HAS
    BEEN ADVISED OF THE POSSIBILITY OR THE DAMAGES ARE FORESEEABLE. TO THE
    FULLEST EXTENT ALLOWED BY LAW, MICROCHIP'S TOTAL LIABILITY ON ALL CLAIMS IN
    ANY WAY RELATED TO THIS SOFTWARE WILL NOT EXCEED THE AMOUNT OF FEES, IF ANY,
    THAT YOU HAVE PAID DIRECTLY TO MICROCHIP FOR THIS SOFTWARE.

    MICROCHIP PROVIDES THIS SOFTWARE CONDITIONALLY UPON YOUR ACCEPTANCE OF THESE
    TERMS.
*/

#include "comman.h"
#include "ble.h"
#include "rtc.h"
#include "led.h"
#include "eeprom.h"
#include "tmr.h"
#include "pin.h"
#include "pwm.h"

/*
                         Main application
 */
void main(void)
{
    // initialize the device
    SYSTEM_Initialize();
    // Enable the Global Interrupts
    INTERRUPT_GlobalInterruptEnable();
    // Enable the Peripheral Interrupts
    INTERRUPT_PeripheralInterruptEnable();

    //init Led
    initLed();
    //init RTC and set TMR0 interrupt callback function
    initRTC();
    TMR0_StartTimer();
	//set TMR2 interrupt callback function
	//start TMR2
	TMR2_StartTimer();
	//initialize BLE Module
    initBLE();

    while (1)
    {
        CLRWDT();
		
		if(eusartRunPara.fMode)
		{
			if((BLE_INT == 0) && BLE_WKP)
			{
                eusartRunPara.nRcvIdx = 0;
				__delay_us(800);
				BLE_WKP	= 0;
			}
			if(BLE_INT && (BLE_WKP == 0))
			{
				BLE_WKP	= 1;
                DecodeDATA();
                eusartRunPara.nRcvIdx = 0;
			}
            if (gLedRunPara.fMin) 
            {
                gLedRunPara.fMin = 0;
                autoRun();
            }

            //every second
            if (gLedRunPara.fSec) 
            {
                gLedRunPara.fSec = 0;
                //when find device
                if (gLedRunPara.find) 
                {
                    findDeviceFlash();
                }
                if (gLedRunPara.fSave) 
                {
                    saveLedPara();
                }
            }
		}
    }
}
/**
 End of File
*/
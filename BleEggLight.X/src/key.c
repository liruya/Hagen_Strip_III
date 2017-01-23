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
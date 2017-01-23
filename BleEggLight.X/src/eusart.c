#include "eusart.h"

unsigned char txBuf[TX_BUF_SIZE] = {0};
unsigned char rxBuf[RX_BUF_SIZE] = {0};

void EUSART_Initialize()
{
    // disable interrupts before changing states
    PIE1bits.RCIE = 0;
    PIE1bits.TXIE = 0;

    // Set the EUSART module to the options selected in the user interface.

    // ABDOVF no_overflow; SCKP Non-Inverted; BRG16 16bit_generator; WUE disabled; ABDEN disabled; 
    BAUD1CON = 0x08;

    // SPEN enabled; RX9 8-bit; CREN enabled; ADDEN disabled; SREN disabled; 
    RC1STA = 0x90;

    // TX9 8-bit; TX9D 0; SENDB sync_break_complete; TXEN enabled; SYNC asynchronous; BRGH hi_speed; CSRC slave; 
    TX1STA = 0x24;

    // Baud Rate = 9600; SP1BRGL 160; 
    SP1BRGL = 0xA0;

    // Baud Rate = 9600; SP1BRGH 1; 
    SP1BRGH = 0x01;

    // enable receive interrupt
    PIE1bits.RCIE = 1;
}

void EUSART_RCV_ISR()
{
    if(1 == RC1STAbits.OERR == 1)
    {
        // EUSART error - restart
        RC1STAbits.CREN = 0;
        RC1STAbits.CREN = 1;
    }
    rxBuf[eusartRunPara.nRcvIdx++] = RCREG;
}

void eusartSendByte(unsigned char byte)
{
    while(!TXIF);
	TXREG = byte;
}

void eusartSendBuffer(const unsigned char *buf)
{
	while(*buf != '\0')
	{
		eusartSendByte(*buf);      
		buf++;
	}
}

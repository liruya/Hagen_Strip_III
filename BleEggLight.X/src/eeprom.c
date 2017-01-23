#include "eeprom.h"

void DATAEE_WriteByte(unsigned int bAdd, unsigned char bData)
{
    unsigned char GIEBitValue = INTCONbits.GIE;

    NVMADRH = ((bAdd >> 8) & 0xFF);
    NVMADRL = (bAdd & 0xFF);
    NVMDATL = bData;    
    NVMCON1bits.NVMREGS = 1;
    NVMCON1bits.WREN = 1;
    INTCONbits.GIE = 0;     // Disable interrupts
    NVMCON2 = 0x55;
    NVMCON2 = 0xAA;
    NVMCON1bits.WR = 1;
    // Wait for write to complete
    while (NVMCON1bits.WR)
    {
    }

    NVMCON1bits.WREN = 0;
    INTCONbits.GIE = GIEBitValue;   // restore interrupt enable
}

unsigned char DATAEE_ReadByte(unsigned int bAdd)
{
    NVMADRH = ((bAdd >> 8) & 0xFF);
    NVMADRL = (bAdd & 0xFF);
    NVMCON1bits.NVMREGS = 1;    
    NVMCON1bits.RD = 1;
    NOP();  // NOPs may be required for latency at high frequencies
    NOP();

    return (NVMDATL);
}

/**
 * write one byte to eeprom @ addr
 * @param addr
 * @param byte
 */
void writeByte( unsigned char addr, unsigned char byte )
{
    DATAEE_WriteByte(EEPROM_START_ADDR + addr, byte);
}

/**
 * read one byte from eeprom @ addr
 * @param addr
 * @return 
 */
unsigned char readByte( unsigned char addr )
{
    unsigned char result = DATAEE_ReadByte(EEPROM_START_ADDR + addr);
    return result;
}

/**
 * write len bytes of buf to eeprom @ addr
 * @param addr
 * @param buf
 * @param len
 */
void writeBuffer( unsigned char addr, unsigned char *buf, unsigned char len )
{
    for (unsigned char i = 0; i < len; i++) 
    {
        writeByte((addr+i), *(buf+i));
    }
}

/**
 * read len bytes from eeprom @ addr to buf
 * @param addr
 * @param buf
 * @param len
 */
void readBuffer( unsigned char addr, unsigned char *buf, unsigned char len )
{
    for (unsigned char i = 0; i < len; i++) 
    {
        *(buf+i) = readByte(addr+i);
    }
}

/**
 * write led para to eeprom @ addr
 * @param addr
 * @param para
 */
void writeManualPara( unsigned char addr, Scene_t para )
{
    writeBuffer(addr, (unsigned char*)&para, sizeof(para));
}

void writeTimerPara( unsigned char addr, unsigned char idx, AutoPara_t para )
{
    writeBuffer(addr+idx*16, (unsigned char*)&para, sizeof(para));
}

/**
 * read the led para from eeprom @ addr
 * @param addr
 * @return 
 */
void readLedPara()
{
    readBuffer(ADDR_MANUAL_PARA, (unsigned char*)&(gLedPara.mPara), sizeof(gLedPara.mPara));
    for (unsigned char i = 0; i < TIMER_COUNT; i++) 
    {
        readBuffer(ADDR_TIMER_PARA+i*16, (unsigned char*)&(gLedPara.aPara[i]), sizeof(gLedPara.aPara[i]) );
    }
}

void saveLedPara()
{
    if (gLedRunPara.nSaveDelayCount > 0) 
    {
        gLedRunPara.nSaveDelayCount--;
    }
    if (gLedRunPara.nSaveDelayCount == 0) 
    {
        writeManualPara( ADDR_MANUAL_PARA, gLedPara.mPara );
        for (unsigned char i = 0; i < TIMER_COUNT; i++) 
        {
            writeTimerPara(ADDR_TIMER_PARA, i, gLedPara.aPara[i]);
        }
        gLedRunPara.fSave = 0;
    }
}
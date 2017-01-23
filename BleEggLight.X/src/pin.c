#include "pin.h"

void PIN_MANAGER_Initialize(void)
{
    /**
    LATx registers
    */   
    LATA = 0x00;    
    LATB = 0x00;
    LATC = 0x00;    

    /**
    TRISx registers
    */    
    TRISA = 0x34;
    TRISB = 0x00;
    TRISC = 0x02;

    /**
    ANSELx registers
    */   
    ANSELC = 0x00;
    ANSELB = 0x00;
    ANSELA = 0x00;
    
    /**
    WPUx registers
    */ 
    WPUB = 0x00;
    WPUA = 0x06;
    WPUC = 0x00;

    /**
    ODx registers
    */   
    ODCONA = 0x00;
    ODCONB = 0x00;
    ODCONC = 0x00;
   
    unsigned char state = GIE;
    GIE = 0;
    PPSLOCK = 0x55;
    PPSLOCK = 0xAA;
    PPSLOCKbits.PPSLOCKED = 0x00; // unlock PPS

    RXPPSbits.RXPPS = 0x11;   //RC1->EUSART:RX;
    RC2PPSbits.RC2PPS = 0x14;   //RC2->EUSART:TX;   
    RC7PPSbits.RC7PPS = 0x0C;   //RC7->CCP1:CCP1;
    RC6PPSbits.RC6PPS = 0x0D;   //RC6->CCP2:CCP2;
    RC3PPSbits.RC3PPS = 0x0E;   //RC3->CCP3:CCP3;
    RC4PPSbits.RC4PPS = 0x0F;   //RC4->CCP4:CCP4;
    RC5PPSbits.RC5PPS = 0x02;   //RC5->PWM5:PWM5;

    PPSLOCK = 0x55;
    PPSLOCK = 0xAA;
    PPSLOCKbits.PPSLOCKED = 0x01; // lock PPS

    GIE = state;
}       

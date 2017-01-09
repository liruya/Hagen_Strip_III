/* 
 * File:   eusart.h
 * Author: liruya
 *
 * Created on 2016?11?19?, ??3:22
 */

#ifndef EUSART_H
#define	EUSART_H

#ifdef	__cplusplus
extern "C" {
#endif

#include "comman.h"

#define TX_BUF_SIZE     32
#define RX_BUF_SIZE     32    

extern unsigned char txBuf[TX_BUF_SIZE];
extern unsigned char rxBuf[RX_BUF_SIZE];
extern volatile EusartRunPara_t eusartRunPara;
    
extern  void EUSART_Initialize();
extern  void EUSART_RCV_ISR();
extern  void eusartSendByte(unsigned char byte);
extern  void eusartSendBuffer(const unsigned char *buf);

#ifdef	__cplusplus
}
#endif

#endif	/* EUSART_H */


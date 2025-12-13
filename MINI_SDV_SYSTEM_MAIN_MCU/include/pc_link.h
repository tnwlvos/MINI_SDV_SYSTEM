/*
 * pc_link.h
 *
 * Created: 2025-12-09 오전 10:54:38
 *  Author: LEESANGHUN
 */ 


#ifndef PC_LINK_H_
#define PC_LINK_H_
#include <stdint.h>
#include "system_state.h"

void PC_Init(void);
void PC_ProcessRx(void);
void PC_ProcessTx(void);
void PC_SendLine(const char *msg);
void PC_OnRxByte(uint8_t data);
void PC_OnTxEmpty(void);



#endif /* PC_LINK_H_ */
/*
 * sub_link.h
 *
 * Created: 2025-12-09 오전 10:54:27
 *  Author: LEESANGHUN
 */ 


#ifndef SUB_LINK_H_
#define SUB_LINK_H_
#include <stdint.h>
#include "system_state.h"

void SUB_Init(void);


//os로 부터 받은 속도 제어 신호 sub에게 송신 함수(uart1)
void SUB_TX_motorcmd(void);

//sub로 부터 초음파 센서 값 수신(uart1)
void SUB_RX_distance(void);

void SUB_OnRxByte(uint8_t data);
void SUB_ONTxEmpty(void);

void SUB_SendRawChar(uint8_t c);
void SUB_SendLine(const char *line);
void SUB_SendToken2(uint8_t a, uint8_t b);
uint8_t SUB_HasLineToPC(void);
void SUB_PopLineToPC(char *out, uint16_t out_sz);
#endif /* SUB_LINK_H_ */


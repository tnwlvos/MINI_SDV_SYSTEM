/*
 * pc_link.c
 *
 * Created: 2025-12-09 오전 10:56:57
 *  Author: LEESANGHUN
 */ 
#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdio.h>
#include "pc_link.h"
#include "hal_uart.h"
#include <string.h>
static char rx_line[64];
static uint8_t rx_idx = 0;
static char tx_line[64];
static uint8_t tx_idx = 0;
static uint8_t tx_len = 0;

static volatile bool line_ready = false;



void PC_Init(void)
{
	HAL_USART1_Init(38400);
	rx_idx = 0;
	tx_idx = 0;
	tx_len=0;
	line_ready = false;
}

void PC_ProcessRx(void)
{
	if (!line_ready) return;

	// TODO: "CMD:..." / "OTA:..." 등의 라인 파싱
	// ex) CMD:PARAM=...
	// 나중에 구현

	rx_idx = 0;
	line_ready = false;
}

void PC_ProcessTx(void)
{
	// TODO: 주기적으로 STATE:MODE=...;MOTOR=...;ULTRA=...
	// 문자열 만들어서 UART0로 전송
	tx_len = sprintf(tx_line,"STATE:ULTRA=%3d;MODE=%d;MOTOR=%d\n",sdv_sys.distance_cm,sdv_sys.mode,sdv_sys.motor_cmd);
	tx_idx = 0;
	HAL_USART1_Enable_Tx_Int();
	
	
}

// ISR glue
void PC_OnRxByte(uint8_t data)
{
	if (data == '\n') {
		rx_line[rx_idx] = '\0';
		line_ready = true;
		} 
	else {
		if (rx_idx < sizeof(rx_line)-1) {
			rx_line[rx_idx++] = data;
		}
	}
}
void PC_ONTxEmpty(void)
{
	
	if(tx_idx<tx_len)
	{
		UDR1=tx_line[tx_idx++];
	}
	else{
		HAL_USART1_Disable_Tx_Int();
		memset(tx_line,0,sizeof(tx_line));
		tx_len=0;
	}
}
ISR(USART1_RX_vect)
{
	uint8_t d = UDR1;
	PC_OnRxByte(d);
}
ISR(USART1_UDRE_vect)
{
	PORTB ^= 0x01;   // 디버깅용
	PC_ONTxEmpty();
	
}
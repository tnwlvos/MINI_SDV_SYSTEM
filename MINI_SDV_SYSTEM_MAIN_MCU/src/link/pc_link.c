/*
 * pc_link.c
 *
 * Created: 2025-12-09 오전 10:56:57
 *  Author: LEESANGHUN
 */ 
#include <avr/io.h>
#include <avr/interrupt.h>
#include "pc_link.h"
#include "hal_uart.h"

static char rx_line[64];
static uint8_t rx_idx = 0;
static volatile bool line_ready = false;
void PC_Init(void)
{
	HAL_USART0_Init(38400);
	rx_idx = 0;
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
}

// ISR glue
void PC_OnRxByte(uint8_t data)
{
	if (data == '\n') {
		rx_line[rx_idx] = '\0';
		line_ready = true;
		} else {
		if (rx_idx < sizeof(rx_line)-1) {
			rx_line[rx_idx++] = data;
		}
	}
}

ISR(USART0_RX_vect)
{
	uint8_t d = UDR0;
	PC_OnRxByte(d);
}
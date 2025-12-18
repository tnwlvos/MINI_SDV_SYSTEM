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
#include "fcw_logic.h"
#include "sub_link.h"
#include "ota_bridge.h"
#include "parameter.h"
#include <string.h>
static char rx_line[256];
static uint8_t rx_idx = 0;
static char tx_line[128];
static uint8_t tx_idx = 0;
static uint8_t tx_len = 0;

static volatile bool line_ready = false;
static volatile uint8_t rx_overflow = 0;


void PC_Init(void)
{
	HAL_USART1_Init(38400);
	rx_idx = 0;
	tx_idx = 0;
	tx_len=0;
	line_ready = false;
}

//Intel HEX형태만 ACK하기 위한 함수
static uint8_t is_hex_char(char c){
	return (c>='0'&&c<='9')||(c>='A'&&c<='F')||(c>='a'&&c<='f');
}

static uint8_t validate_ihex_line(const char *p){
	if (!p || p[0] != ':') return 0;
	size_t L = strlen(p);
	if (L < 2) return 0;        // 최소 레코드 길이
	if (L > 96) return 0;        // 너 시스템에서 상한(128보다 작게)
	for (size_t i=1;i<L;i++){
		if (!is_hex_char(p[i])) return 0;
	}
	return 1;
}


void PC_ProcessRx(void)
{
	if (!line_ready) return;

	if (rx_overflow) {
		rx_overflow = 0;
		rx_idx = 0;
		line_ready = false;
		PC_SendLine("OTA:NAK:RX_OVERFLOW");
		return;
	}
	 // \r 제거 (윈도우 터미널 대비)
	size_t n = strlen(rx_line);
	if (n > 0 && rx_line[n-1] == '\r') rx_line[n-1] = '\0';

	if (strncmp(rx_line, "OTA:BEGIN:MAIN", 14) == 0) {
		OTA_Bridge_Begin(OTA_TARGET_MAIN);
	}
	else if (strncmp(rx_line, "OTA:BEGIN:SUB", 13) == 0) {
		OTA_Bridge_Begin(OTA_TARGET_SUB);
	}
	else if (strncmp(rx_line, "OTA:END", 7) == 0) {
		OTA_Bridge_End();
	}
	else if (rx_line[0] == ':') {
		 //  "OTA:DATA:" 없이 순수 HEX 라인
		 if (sdv_sys.ota_active) {
			 OTA_Bridge_Data(rx_line);
			 } 
		else{
			 PC_SendLine("OTA:NAK:NOT_IN_SUB_OTA");
		 }
	 }
	
	else {
		// 일반 커맨드 처리(나중에)
		PC_SendLine("DBG:UNKNOWN CMD");
	}
	 rx_idx = 0;
	 line_ready = false;
}

void PC_ProcessTx(void)
{
	// TODO: 주기적으로 STATE:MODE=...;MOTOR=...;ULTRA=...
	// 문자열 만들어서 UART0로 전송
	if (sdv_sys.ota_active) return;  
	uint16_t ttc10;
	if (tx_len != 0) return; // 전송 중이면 무시

	
	cli();
	ttc10=sdv_sys.ttc*10;
	tx_len = sprintf(tx_line,
	"STATE:ULTRA=%3d;MODE=%d;MOTOR=%d;Speed=%d;FCW=%d;TTC=%2u.%1u\n",
	(uint16_t)sdv_sys.distance_cm,
	sdv_sys.mode,
	sdv_sys.motor_cmd,
	(int)sdv_sys.speed_cms,
	(unsigned int)sdv_sys.fcw_state,
	(ttc10/10),
	(ttc10%10));
	

	tx_idx = 0;
	HAL_USART1_Enable_Tx_Int();

	sei();
}

// ISR glue
void PC_OnRxByte(uint8_t data)
{
	if (data == '\r') return; // CR 제거

	if (data == '\n') {
		rx_line[rx_idx] = '\0';
		line_ready = true;
		return;
	}

	if (rx_overflow) {
		// overflow 상태면 newline 나올 때까지 버림
		return;
	}

	if (rx_idx < sizeof(rx_line) - 1) {
		rx_line[rx_idx++] = data;
		} else {
		// 버퍼 꽉 참 → overflow 상태
		rx_overflow = 1;
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
		tx_idx=0;
		tx_len=0;
	}
}

void PC_SendLine(const char *msg)
{
	while (tx_len != 0) { /* wait until previous send done */ }
	tx_len = snprintf(tx_line, sizeof(tx_line), "%s\n", msg);
	tx_idx = 0;
	HAL_USART1_Enable_Tx_Int();
}


ISR(USART1_RX_vect)
{
	uint8_t d = UDR1;
	PC_OnRxByte(d);
}
ISR(USART1_UDRE_vect)
{
	
	PC_ONTxEmpty();
	
}
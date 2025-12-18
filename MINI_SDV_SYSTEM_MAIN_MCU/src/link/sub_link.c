/*
 * sub_link.c
 *
 * Created: 2025-12-09 오전 10:56:48
 *  Author: LEESANGHUN
 */ 
#include <avr/io.h>
#include <avr/interrupt.h>
#include "sub_link.h"
#include "hal_uart.h"
#include "lcd_gcc.h"
#include "pc_link.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <util/atomic.h>
static volatile char sub_ota_rx_buf[256];
static volatile uint8_t sub_ota_rx_idx = 0;
static volatile uint8_t sub_ota_rx_len =0;

static volatile uint8_t sub_line_ready = 0;
static char sub_line_to_pc[256];

static volatile uint8_t srf_buf[2];
static volatile uint8_t srf_idx = 0;

static volatile uint8_t tx_buf[2];   // 보낼 motor_cmd
static volatile uint8_t tx_idx = 0;

static char sub_tx_buf[192];
static const char *sub_tx_ptr = NULL;

static volatile uint8_t sub_tx_busy = 0;

void SUB_Init(void){
	HAL_USART0_Init(38400);
	srf_idx=0;
}

void SUB_TX_motorcmd()
{
	static uint8_t last_fcw_state=FCW_SAFE;
	static uint8_t init = 0;

	if (!init) {
		last_fcw_state = sdv_sys.fcw_state;
		sdv_sys.last_motor_cmd = sdv_sys.motor_cmd;
		init = 1;
		
	}

	if((sdv_sys.motor_cmd != sdv_sys.last_motor_cmd) || sdv_sys.fcw_state !=last_fcw_state)
	{
		tx_buf[0]=(uint8_t)sdv_sys.motor_cmd;
		tx_buf[1] =(uint8_t)sdv_sys.fcw_state;
		tx_idx=0;
		HAL_USART0_Enable_Tx_Int();
		sdv_sys.last_motor_cmd=sdv_sys.motor_cmd;
		last_fcw_state=sdv_sys.fcw_state;
	}
	
}

void SUB_OnRxByte(uint8_t data){
	
	if (sdv_sys.ota_target == OTA_TARGET_SUB) {

		// CR 제거(윈도우)
		if (data == '\r') return;

		// 개행이면 한 줄 완성
		if (data == '\n') {
			sub_ota_rx_buf[sub_ota_rx_idx] = '\0';   //문자열 종료
			snprintf(sub_line_to_pc, sizeof(sub_line_to_pc), "%s", (char*)sub_ota_rx_buf);
			sub_line_ready = 1;
			sub_ota_rx_idx = 0;
			return;
		}

		// 버퍼 저장 (오버플로 방지)
		if (sub_ota_rx_idx < sizeof(sub_ota_rx_buf) - 1) {
			sub_ota_rx_buf[sub_ota_rx_idx++] = (char)data;
			} 
		else {
			// 너무 길면 리셋(또는 NAK)
			sub_ota_rx_idx = 0;
			PC_SendLine("OTA:NAK:SUB_RX_OVERFLOW");
		}
		return;
	}

	// ===== 평상시: 거리값 2바이트 수신 =====
	srf_buf[srf_idx++] = data;
	if (srf_idx >= 2) {
		srf_idx = 0;
		sdv_sys.distance_cm = ((uint16_t)srf_buf[1] << 8) | srf_buf[0];
		sdv_sys.distance_flag = true;
	}
}

void SUB_SendLine(const char *line)
{
	// 송신 중이면 기다림(짧게)
	while (sub_tx_busy) {;}

	// 내부 버퍼에 복사 (깨짐 방지)
	strncpy(sub_tx_buf, line, sizeof(sub_tx_buf)-1);
	sub_tx_buf[sizeof(sub_tx_buf)-1] = '\0';

	sub_tx_ptr  = sub_tx_buf;
	sub_tx_busy = 1;

	sub_proto_mode = SUB_PROTO_OTA_TEXT;   //  텍스트로 보낼 때는 모드 보장
	HAL_USART0_Enable_Tx_Int();
}

void SUB_OnTxEmpty(void)
{
	if (sub_tx_ptr && *sub_tx_ptr) {
		UDR0 = *sub_tx_ptr++;
		} 
	else {
		UDR0 = '\n';
		sub_tx_ptr = NULL;
		sub_tx_busy = 0;
		HAL_USART0_Disable_Tx_Int();
	}
}

void SUB_SendToken2(uint8_t a, uint8_t b)
{
	tx_buf[0] = a;
	tx_buf[1] = b;
	tx_idx = 0;
	sub_proto_mode = SUB_PROTO_BINARY;
	HAL_USART0_Enable_Tx_Int();
}

uint8_t SUB_HasLineToPC(void)
{
	return sub_line_ready;
}

void SUB_PopLineToPC(char *out, uint16_t out_sz)
{
	if (!out || out_sz == 0) return;

	ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
		if (!sub_line_ready) { out[0] = '\0'; return; }

		strncpy(out, sub_line_to_pc, out_sz - 1);
		out[out_sz - 1] = '\0';
		sub_line_ready = 0;
	}
	
}

ISR(USART0_RX_vect)
{
	uint8_t data= UDR0;
	SUB_OnRxByte(data);
}
ISR(USART0_UDRE_vect)
{
	if (sub_proto_mode == SUB_PROTO_OTA_TEXT) {
		SUB_OnTxEmpty();
		} 
	else {
		// 기존 2바이트 binary 전송
		UDR0 = tx_buf[tx_idx++];
		if (tx_idx >= 2) {
			tx_idx = 0;
			HAL_USART0_Disable_Tx_Int();
		}
	}
}
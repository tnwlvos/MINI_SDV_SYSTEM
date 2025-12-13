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

static volatile uint8_t srf_buf[2];
static volatile uint8_t srf_idx = 0;
static volatile uint8_t tx_buf[2];   // 보낼 motor_cmd
static volatile uint8_t tx_idx = 0;

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
	srf_buf[srf_idx++]=data;
	if(srf_idx>=2)
	{
		srf_idx=0;
		sdv_sys.distance_cm=(srf_buf[1]<<8)|srf_buf[0];
		sdv_sys.distance_flag=true;
		
	}
}
void SUB_ONTxEmpty(void)
{
	UDR0=tx_buf[tx_idx++];
	if(tx_idx>=2){
		tx_idx=0;
		HAL_USART0_Disable_Tx_Int();	
	}
}

ISR(USART0_RX_vect)
{
	uint8_t data= UDR0;
	SUB_OnRxByte(data);
}
ISR(USART0_UDRE_vect)
{
	
	SUB_ONTxEmpty();
	
}
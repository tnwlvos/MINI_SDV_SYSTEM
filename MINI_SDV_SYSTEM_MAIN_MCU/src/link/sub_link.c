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

static volatile uint8_t srf_buf[2];
static volatile uint8_t srf_idx = 0;
static volatile uint8_t tx_buf;   // 보낼 motor_cmd

void SUB_Init(void){
	HAL_USART1_Init(38400);
	srf_idx=0;
}

void SUB_TX_motorcmd()
{
	if(sdv_sys.motor_cmd != sdv_sys.last_motor_cmd)
	{
		tx_buf=(uint8_t)sdv_sys.motor_cmd;
		HAL_USART1_Enable_Tx_Int();
		sdv_sys.last_motor_cmd=sdv_sys.motor_cmd;
	}
	
}
void SUB_RX_distance(void)
{
	
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
	UDR1=tx_buf;
	HAL_USART1_Disable_Tx_Int();
}

ISR(USART1_RX_vect)
{
	uint8_t data= UDR1;
	SUB_OnRxByte(data);
}
ISR(USART1_UDRE_vect)
{
	SUB_ONTxEmpty();
	
}
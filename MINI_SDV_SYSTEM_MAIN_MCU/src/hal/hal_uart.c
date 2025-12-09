/*
 * hal_uart.c
 *
 * Created: 2025-12-09 오전 10:55:59
 *  Author: LEESANGHUN
 */ 
#include<avr/io.h>
#include "hal_uart.h"
#define F_CPU 14745600UL
void HAL_USART0_Init(uint32_t baud){
	const uint16_t ubrr0 = (F_CPU/(16UL*baud)) - 1;
	UBRR0H = (uint8_t)(ubrr0 >> 8);
	UBRR0L = (uint8_t)(ubrr0 & 0xFF);
	UCSR0A = 0x00;
	UCSR0B = (1<<RXCIE0) | (1<<RXEN0) | (1<<TXEN0);  // RX IRQ, RX/TX enable
	UCSR0C = (1<<UCSZ01) | (1<<UCSZ00);              // 8N1
}
/* ================= UART1 ================= */
void HAL_USART1_Init(uint32_t baud){
	const uint16_t ubrr1 = (F_CPU/(16UL*baud)) - 1;
	UBRR1H = (uint8_t)(ubrr1 >> 8);
	UBRR1L = (uint8_t)(ubrr1 & 0xFF);
	UCSR1A = 0x00;
	UCSR1B = (1<<RXCIE1) | (1<<RXEN1) | (1<<TXEN1);  // RX IRQ, RX/TX enable
	UCSR1C = (1<<UCSZ11) | (1<<UCSZ10);              // 8N1
}

void HAL_USART1_Enable_Tx_Int(void){UCSR1B |= (1<<UDRIE1);}
void HAL_USART1_Disable_Tx_Int(void){UCSR1B &= ~(1<<UDRIE1);}
void HAL_USART0_Enable_Tx_Int(void){UCSR0B |= (1<<UDRIE0);}
void HAL_USART0_Disable_Tx_Int(void){UCSR0B &= ~(1<<UDRIE0);}
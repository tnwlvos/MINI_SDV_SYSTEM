/*
 * hal_uart.h
 *
 * Created: 2025-12-09 오전 10:53:31
 *  Author: LEESANGHUN
 */ 


#ifndef HAL_UART_H_
#define HAL_UART_H_

#include<stdint.h>

void HAL_USART0_Init(uint32_t baud);
void HAL_USART1_Init(uint32_t baud);

//송신 인터럽트 enable/disable
void HAL_USART1_Enable_Tx_Int(void);
void HAL_USART1_Disable_Tx_Int(void);
void HAL_USART0_Enable_Tx_Int(void);
void HAL_USART0_Disable_Tx_Int(void);

#endif /* HAL_UART_H_ */
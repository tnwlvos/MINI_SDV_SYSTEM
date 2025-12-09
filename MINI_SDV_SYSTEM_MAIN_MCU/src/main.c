/*
 * MINI_SDV_SYSTEM_MAIN_MCU.c
 *
 * Created: 2025-11-11 오전 10:41:36
 * Author : LEESANGHUN
 */ 

#define F_CPU 14745600UL
#define BAUD 38400UL
// ==============헤더 파일=====================
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include "lcd_gcc.h"
#include "usart_gcc.h"

//================파라미터 설정===================


#define SPEED_UP 1
#define SPEED_DOWN 2
#define MOTOR_STOP 3
#define SPEED_STAY 4

static volatile uint8_t srf_buf[2];
static volatile uint8_t srf_buf_idx=0;
static volatile uint8_t speed_buf;
volatile bool rx_complete_flag= false;

//=================함수 초기화===============

static void usart0_init(void);
static void usart1_init(void);
void send_ultra_to_os_uart0();
void send_speed_to_sub_uart1();
void sub_rx_ultrasonic_uart1();
unsigned char os_rx_speedcmd_uart0();
unsigned char send_ota_to_sub_uart1();



/* ================= UART0 ================= */
static void usart0_init(void){
	const uint16_t ubrr0 = (F_CPU/(16UL*BAUD)) - 1;
	UBRR0H = (uint8_t)(ubrr0 >> 8);
	UBRR0L = (uint8_t)(ubrr0 & 0xFF);
	UCSR0A = 0x00;
	UCSR0B = (1<<RXCIE0) | (1<<RXEN0) | (1<<TXEN0);  // RX IRQ, RX/TX enable
	UCSR0C = (1<<UCSZ01) | (1<<UCSZ00);              // 8N1
}
/* ================= UART1 ================= */
static void usart1_init(void){
	const uint16_t ubrr1 = (F_CPU/(16UL*BAUD)) - 1;
	UBRR1H = (uint8_t)(ubrr1 >> 8);
	UBRR1L = (uint8_t)(ubrr1 & 0xFF);
	UCSR1A = 0x00;
	UCSR1B = (1<<RXCIE1) | (1<<RXEN1) | (1<<TXEN1);  // RX IRQ, RX/TX enable
	UCSR1C = (1<<UCSZ11) | (1<<UCSZ10);              // 8N1
}


//sub로 부터 받은 초음파 센서 값 os에게 송신 함수(uart0)
//void send_ultra_to_os_uart0(){
///*  
//필요 인수: 초음파 센서 값
//반환 값: 굳이 필요 X
//구현 필요 내용: 받은 초음파 센서 값을 형변환하여 전송(상위,하위 바이트 변환 필요)
			   //인터럽트를 사용 해 전송할 것이니 UDRIE0 활성화 필요
			   //(인터럽트 루틴 안에서 송신 후 비활성화 필요)
//*/	
//}


//os로 부터 받은 속도 제어 신호 sub에게 송신 함수(uart1)
void send_speed_to_sub_uart1(uint8_t *speed_flag){
///*
//필요 인수: os로 부터 받은 속도 제어 신호
//반환 값:굳이 필요 X
//구현 필요 내용: os로 부터 받은 속도 제어 신호를 전송
			   //인터럽트를 사용 해 전송할 것이니 UDRIE1 활성화 필요
			   //(인터럽트 루틴 안에서 송신 후 비활성화 필요)
//*/	
	speed_buf=*speed_flag;
	UCSR1B |= (1<<UDRIE1);

}


//sub로 부터 초음파 센서 값 수신(uart1)
void sub_rx_ultrasonic_uart1(unsigned int *dis){
///*
//필요 인수 : 없음
//반환 값: 초음파 센서 값 
//구현 필요 내용: 수신 인터럽트를 통해 받은 udr값을 초음파 센서 값 변수에 저장
//*/	
	
	*dis=(srf_buf[1]<<8) | srf_buf[0];
	
	

}


//os로 부터 속도 제어 신호 수신(uart0)
//unsigned char os_rx_speedcmd_uart0(){
///*
//필요 인수: 없음
//반환 값: 속도 제어 신호
//구현 필요 내용:수신 인터럽트를 통해 받은 udr값을 속도 제어 신호 변수에 저장
//*/	
//}



//os로 부터 ota 신호 및 값 수신(uart0)
//unsigned char os_rx_ota_uart0(){
///*
//필요 인수: 없음
//반환 값: ota 신호, 변경 parameter값
//
//구현 필요 내용:수신 인터럽트를 통해 받은 udr값을 각각 ota신호 변수, 변경 parameter 값으로 저장
//*/	
//}



//sub에게 os로 부터 받은 ota 신호 및 값 송신(uart1)
//unsigned char send_ota_to_sub_uart1()
//{
///*
//필요 인수: ota신호
//반환 값: 없음
//구현 필요 내용: ota 신호값을 인수로 받아 uart1을 통해 송신함
	//인터럽트를 사용 해 전송할 것이니 UDRIE1 활성화 필요
	//(인터럽트 루틴 안에서 송신 후 비활성화 필요)
//*/	
	//
//}

/*===============_PC(OS)와 송수신==========*/
//uart0 데이터 전송 인터럽트
//ISR(UART0_UDRE_vect){
	//
//}
//
//
////uart0 데이터 수신 인터럽트
//ISR(UART0_RX_vect){
	//
//}
//


/*===============SUB_MCU와 송수신==========*/
//uart1 데이터 전송 인터럽트
ISR(USART1_UDRE_vect){
	UDR1=speed_buf;
	
	UCSR1B &= ~(1<<UDRIE1);
	
	
}
//
////uart1 데이터 수신 인터럽트
ISR(USART1_RX_vect){
	 uint8_t data = UDR1;         
	 srf_buf[srf_buf_idx++] = data;

	if(srf_buf_idx>=2){
		srf_buf_idx=0;
		rx_complete_flag=true;
		
	}
	
}


int main(void)
{

	usart0_init();
	usart1_init();
	LCD_Init();
	char Message[40];
	unsigned int distance=0;
	sei();
	DDRD=0x00;
	uint8_t mode;
	uint8_t motor_mode;
	uint8_t last_motor_mode = SPEED_STAY;
	while (1)
	{
		
		// 버튼 읽기 (눌리면 1 되도록 반전)
		uint8_t key = (~PIND) & 0x0F;  // PD0~PD3만 사용

		uint8_t motor_mode = last_motor_mode;

		if (key & (1<<0))       motor_mode = SPEED_UP;    // SW0
		else if (key & (1<<1))  motor_mode = SPEED_DOWN;  // SW1
		else if (key & (1<<2))  motor_mode = SPEED_STAY;  // SW2
		else if (key & (1<<3))  motor_mode = MOTOR_STOP;  // SW3
		else {
			// 아무 버튼도 안 눌림 → 이전 상태 유지 (SPEED_STAY 느낌)
		}

		// 값이 바뀐 경우에만 서브로 전송 (괜히 계속 보내지 말고)
		if (motor_mode != last_motor_mode) {
			send_speed_to_sub_uart1(&motor_mode);
			last_motor_mode = motor_mode;
		}

		if(rx_complete_flag)
		{
			rx_complete_flag=false;
			sub_rx_ultrasonic_uart1(&distance);
			LCD_Pos(0,0);
			LCD_Str("Measured Dist=");
			sprintf(Message,"%01d   %03d cm",sizeof(distance) ,distance);
			LCD_Pos(1,5);
			LCD_Str(Message);
			
		}
		
	}
}

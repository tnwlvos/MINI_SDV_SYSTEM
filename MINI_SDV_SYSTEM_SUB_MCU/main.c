/*
 * MINI_SDV_SYSTEM_SUB_MCU.c
 *
 * Created: 2025-11-11 오전 10:41:09
 * Author : LEESANGHUN
 */ 


#define F_CPU 14745600UL
#define BAUD 38400UL
#define _USE_SAFTY_TWI_ 1
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
#include "srf02_utils.h"



//================파라미터 설정===================

#define SPEED_UP 1
#define SPEED_DOWN 2
#define MOTOR_STOP 3
#define SPEED_STAY 4
#define FORWARD 0
#define BACKWARD 1

#define MOTOR_SPEED_basic 200
#define MOTOR_SPEED_decrease 150
static volatile uint8_t speedflag_buf;


#define IN1_A   PD4
#define IN2_A   PD5
// B
#define IN1_B   PD6
#define IN2_B   PD7
// C
#define IN1_C   PB0
#define IN2_C   PB1
// D
#define IN1_D   PB2
#define IN2_D   PB3


//==============전역변수============
volatile uint16_t ti_Cnt_1ms;
volatile unsigned char measure_ready;
static volatile uint8_t srf_buf[2];
static volatile uint8_t srf_buf_idx=0;

//속도
volatile uint8_t speedA=150;
volatile uint8_t speedB=150;
volatile uint8_t speedC=150;
volatile uint8_t speedD=150;


volatile bool rx_complete_flag= false;

//=================함수 초기화===============
//static void usart1_init(void);
//void send_ultra_to_sub_uart1();
//unsigned char main_rx_speedcmd_uart1();
//unsigned char main_rx_ota_uart1();
//void eeprom_update_param();
//void motor_drive();
//


/* ================= UART1 ================= */
static void usart1_init(void){
	const uint16_t ubrr = (F_CPU/(16UL*BAUD)) - 1;
	UBRR1H = (uint8_t)(ubrr >> 8);
	UBRR1L = (uint8_t)(ubrr & 0xFF);
	UCSR1A = 0x00;
	UCSR1B = (1<<RXCIE1) | (1<<RXEN1) | (1<<TXEN1);  // RX IRQ, RX/TX enable
	UCSR1C = (1<<UCSZ11) | (1<<UCSZ10);              // 8N1
}
//=================타이머카운터0==================
void Timer0_Init(){//1ms
	TCCR0=(1<<WGM01)|(1<<CS00)|(1<<CS01)|(1<<CS02);
	TCNT0=0x00;
	OCR0=14;
	TIMSK=(1<<OCIE0);
}

//=================타이머카운터1==================
void pwm1_init(void){
	// Timer1 Fast PWM 8bit
	TCCR1A = (1<<COM1A1)|(1<<COM1B1)|(1<<WGM10);
	TCCR1B = (1<<WGM12)|(1<<CS11); // prescaler 8

	
}
//=================타이머카운터3==================
void pwm3_init(void){
	// Timer3 Fast PWM 8bit
	TCCR3A = (1<<COM3A1)|(1<<COM3B1)|(1<<WGM30);
	TCCR3B = (1<<WGM32)|(1<<CS31); // prescaler 8

}



void motor_speed_set(uint8_t speed){
	OCR1A=speed;
	OCR1B =speed;
	OCR3A=speed;
	OCR3B =speed;
	
}
//main_mcu에게 초음파 센서 값 송신(uart1)
void send_ultra_to_sub_uart1(unsigned int *range){
///*  
//필요 인수: 초음파 센서 값
//반환 값: 굳이 필요 X
//구현 필요 내용: 받은 초음파 센서 값을 형변환하여 전송(상위,하위 바이트 변환 필요)
			   //인터럽트를 사용 해 전송할 것이니 UDRIE1 활성화 필요
			   //(인터럽트 루틴 안에서 송신 후 비활성화 필요)*/
	srf_buf[1]=(*range>>8)&0xff;
	srf_buf[0]=(*range&0xff);
	srf_buf_idx=0;
	UCSR1B |= (1<<UDRIE1);
	

}

//main_mcu로 부터 속도 제어 신호 수신(uart1)
void main_rx_speedcmd_uart1(uint8_t *motor_flag){
/*  
필요 인수: X
반환 값: 속도 제어 신호
구현 필요 내용: main_mcu로부터 받은 제어 신호를 수신함		  
*/	
	*motor_flag=speedflag_buf;

}

//ota 변경 parameter값 수신(uart1)
//unsigned char main_rx_ota_uart1(){
///*  
//필요 인수: X
//반환 값: parameter값 
//구현 필요 내용:  main_mcu로부터 받은 parameter 값을 수신함
//*/	
//
//
//}


//eeprom 수정(parameter 값 수정)
//void eeprom_update_param(){
///*  
//필요 인수: ota신호, parameter값
//반환 값: 굳이 필요 X
//구현 필요 내용: ota신호와 parameter 값을 기반으로 sub_mcu의 parameter값을 변경함
//*/	
//
//
//}


//초음파 센서값 받아오기



void motor_driection(uint8_t dir_flag){
	  if (dir_flag == FORWARD)
	  {
		 // A : PD0 = 1, PD1 = 0
		 PORTD |=  (1<<IN1_A);
		 PORTD &= ~(1<<IN2_A);

		 // B : PD2 = 1, PD3 = 0
		 PORTD |=  (1<<IN1_B);
		 PORTD &= ~(1<<IN2_B);

		 // C : PB0 = 1, PB1 = 0
		 PORTB |=  (1<<IN1_C);
		 PORTB &= ~(1<<IN2_C);

		 // D : PB2 = 1, PB3 = 0
		 PORTB |=  (1<<IN1_D);
		 PORTB &= ~(1<<IN2_D);
		}
	  else   // BACKWARD
	  {
		 
		 PORTD &= ~(1<<IN1_A);
		 PORTD |=  (1<<IN2_A);

	
		 PORTD &= ~(1<<IN1_B);
		 PORTD |=  (1<<IN2_B);

	
		 PORTB &= ~(1<<IN1_C);
		 PORTB |=  (1<<IN2_C);

	
		 PORTB &= ~(1<<IN1_D);
		 PORTB |=  (1<<IN2_D);
		}
}

//서브 모터 구동
void motor_drive(uint8_t flag, uint8_t *speed){
	if (flag==SPEED_UP){
		*speed=MOTOR_SPEED_basic;
		motor_speed_set(*speed);
		
	}
	else if (flag==SPEED_DOWN){
		*speed=MOTOR_SPEED_decrease;
		motor_speed_set(*speed);
		
	}
	else if(flag == SPEED_STAY){
		
		motor_speed_set(*speed);
	}

	else if(flag==MOTOR_STOP){
		for(int16_t s  = *speed; s >= 0; s -= 10){
			OCR1A = s;
			OCR1B = s;
			OCR3A = s;
			OCR3B = s;
			_delay_ms(3);
		}
		 motor_speed_set(0);
		*speed=0;
		// IN 핀 모두 LOW (브레이크 해제)
		PORTD &= ~((1<<IN1_A)|(1<<IN2_A)|(1<<IN1_B)|(1<<IN2_B));
		PORTB &= ~((1<<IN1_C)|(1<<IN2_C)|(1<<IN1_D)|(1<<IN2_D));
	}
	
}


//펌웨어 수정(구현 가능 할 시)



/*===============MAIN_MCU와 송수신==========*/
//uart1 데이터 전송 인터럽트
ISR(USART1_UDRE_vect){
	//하위 부터전송
	UDR1=srf_buf[srf_buf_idx++];
	
	if(srf_buf_idx>=2){
		UCSR1B &= ~(1<<UDRIE1);
		srf_buf_idx=0;
	}
	
	
}

//uart1 데이터 수신 인터럽트
ISR(USART1_RX_vect){
	 speedflag_buf = UDR1;
	 
	 rx_complete_flag=true;
}

// 타이머카운터0 isr
ISR(TIMER0_COMP_vect){
	ti_Cnt_1ms++;
	if(ti_Cnt_1ms>=100){
		measure_ready=1;
		ti_Cnt_1ms=0;
	}
}
void disable_jtag()
{
	MCUCSR |= (1<<JTD);
	MCUCSR |= (1<<JTD);
	MCUCSR |= (1<<JTD);
	MCUCSR |= (1<<JTD);
}

int main(void)
{
	disable_jtag();
	uint8_t speed=MOTOR_SPEED_basic;
	usart1_init();
	LCD_Init();
	Timer0_Init();
	Init_TWI();	
	pwm1_init();
	pwm3_init();
	motor_speed_set(speed);

	
	/* PWM 핀 */
	DDRB |= (1<<PB5) | (1<<PB6) | (1<<PB0) | (1<<PB1) | (1<<PB2) | (1<<PB3);
	DDRE |= (1<<PE3) | (1<<PE4);
	DDRD |= (1<<PD4) | (1<<PD5) | (1<<PD6) | (1<<PD7);
	char Sonar_Addr=SRF02_ADDR_0;
	unsigned int Sonar_range;
	char Message[40];
	int readCnt=0;
	unsigned int res=0;
	uint8_t motor_flag=0;
	uint8_t motor_dir_flag=FORWARD;



		
	uint8_t ret = SRF02_i2C_Write(0xE0, 0, 0x51);
	LCD_Pos(1,0);
	sprintf(Message, "W:%d", ret);
	LCD_Str(Message);
	
	startRanging(Sonar_Addr);
	ti_Cnt_1ms=0;
	
	
	sei();
	motor_driection(FORWARD);
	motor_drive(MOTOR_SPEED_basic, &speed);
	while (1)
	{
		
		if(rx_complete_flag){
			main_rx_speedcmd_uart1(&motor_flag);
			
			rx_complete_flag=false;
		}
		
		motor_driection(FORWARD);
		motor_drive(motor_flag,&speed);
		
		if(measure_ready==1){
			measure_ready=0;
			
		
			res = getRange(Sonar_Addr, &Sonar_range);
			
			if(res !=0){
				LCD_Pos(0,0);
				LCD_Str("Measured Dist=");
				LCD_Pos(1,5);
				LCD_Str("ERR           ");
				continue;
			}
			LCD_Pos(0,0);
			LCD_Str("Measured Dist=");
			sprintf(Message,"%01d   %03d cm",sizeof(Sonar_range) ,Sonar_range);
			LCD_Pos(1,5);
			LCD_Str(Message);
			
			startRanging(Sonar_Addr);
			send_ultra_to_sub_uart1(&Sonar_range);
			readCnt=(readCnt+1)%10;
		}
		
	
	}
	
		
}
	
	
	

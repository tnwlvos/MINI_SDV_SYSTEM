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
//motor_cmd_flag
#define SPEED_UP 1
#define SPEED_DOWN 2
#define MOTOR_STOP 3
#define SPEED_STAY 4

//fcw_flag
#define FCW_SAFE     0
#define FCW_WARNING  1
#define FCW_DANGER   2
#define FCW_ERROR    3

//motor_dir flag
#define FORWARD 0
#define BACKWARD 1
//motor speed 
#define MOTOR_SPEED_basic 200
#define MOTOR_SPEED_decrease 150




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


#define BUZZ_PIN_BIT   4   // PORTG0

#define BUZ_TICK_HZ    10000UL
#define MS_TO_TICKS(ms) ((uint32_t)(ms) * (BUZ_TICK_HZ/1000UL))
//==============전역변수============
volatile uint16_t ti_Cnt_1ms;
volatile unsigned char measure_ready;
static volatile uint8_t srf_buf[2];
static volatile uint8_t srf_buf_idx=0;
volatile uint8_t rx_buf[2];
volatile uint8_t rx_idx=0;

//속도
volatile uint8_t speedA=150;
volatile uint8_t speedB=150;
volatile uint8_t speedC=150;
volatile uint8_t speedD=150;

//flag bufer
static volatile uint8_t speedflag_buf;
static volatile uint8_t fcw_state_buf;


volatile bool rx_complete_flag= false;

// ===== buzzer core =====
static volatile uint16_t buz_half_ticks = 0;
static volatile uint16_t buz_toggle_cnt = 0;
static volatile uint8_t  buz_enable = 0;

// ===== WARNING double-beep FSM =====
static volatile uint8_t  buz_pattern = 0;   // 0=continuous, 1=pattern
static volatile uint16_t buz_phase_cnt = 0;
static volatile uint8_t  warn_step = 0;     // 0:ON1,1:OFF1,2:ON2,3:OFF_LONG

/* ================= UART1 ================= */
static void usart0_init(void){
	const uint16_t ubrr = (F_CPU/(16UL*BAUD)) - 1;
	UBRR0H = (uint8_t)(ubrr >> 8);
	UBRR0L = (uint8_t)(ubrr & 0xFF);
	UCSR0A = 0x00;
	UCSR0B = (1<<RXCIE0) | (1<<RXEN0) | (1<<TXEN0);  // RX IRQ, RX/TX enable
	UCSR0C = (1<<UCSZ01) | (1<<UCSZ00);              // 8N1
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
//=============== 부저용 타이머카운터 2===============
void Timer2_Init(){
	  
	 // prescaler = 64
	 TCCR2 = (1<<WGM21) | (1<<CS22);

	 // 2kHz tick
	 // OCR2 = F_CPU/(64*2000) - 1 ≈ 114
	 OCR2 = 114;

	   // 비교매치 인터럽트 enable
	   TIMSK |= (1<<OCIE2);
}
static uint16_t buz_calc_half_ticks(uint16_t freq_hz)
{
	if (freq_hz == 0) return 0;

	// half_ticks = tick_hz / (2*freq)
	uint32_t ht = (BUZ_TICK_HZ + (freq_hz)) / (2UL * freq_hz); // 반올림 느낌
	if (ht < 1) ht = 1;
	if (ht > 60000) ht = 60000;
	return (uint16_t)ht;
}






void motor_speed_set(uint8_t speed){
	OCR1A=speed;
	OCR1B =speed;
	OCR3A=speed;
	OCR3B =speed;
	
}
//main_mcu에게 초음파 센서 값 송신(uart1)
void send_ultra_to_sub_uart0(unsigned int *range){
///*  
//필요 인수: 초음파 센서 값
//반환 값: 굳이 필요 X
//구현 필요 내용: 받은 초음파 센서 값을 형변환하여 전송(상위,하위 바이트 변환 필요)
			   //인터럽트를 사용 해 전송할 것이니 UDRIE1 활성화 필요
			   //(인터럽트 루틴 안에서 송신 후 비활성화 필요)*/
	srf_buf[1]=(*range>>8)&0xff;
	srf_buf[0]=(*range&0xff);
	srf_buf_idx=0;
	UCSR0B |= (1<<UDRIE0);
	

}

//main_mcu로 부터 속도 제어 신호, fcw 상태 수신(uart0)
void main_rx_cmd_uart0(uint8_t *motor_flag, uint8_t *fcw_state){
/*  
필요 인수: X
반환 값: 속도 제어 신호
구현 필요 내용: main_mcu로부터 받은 제어 신호를 수신함		  
*/	
	*motor_flag=speedflag_buf;
	*fcw_state=fcw_state_buf;
}

void Ultrasonic_filtered(unsigned int *range){
	static uint16_t prev_dfiltered = 0;
	if(*range == 0 || *range > 500){
		*range = prev_dfiltered;
		return;
	}
	
	
	if(prev_dfiltered == 0){
		prev_dfiltered = *range;
		return;
	}

	// 필터 계수 0.25(0 < alpha < 1)
	*range=( (*range)+3*prev_dfiltered) / 4;
	prev_dfiltered = *range;
	
	
}

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

void fcw_state_to_string(uint8_t state){
	LCD_Pos(1,0);
	switch(state){
		case FCW_SAFE:
			LCD_Str("state=SAFE    ");
			break;
		case FCW_WARNING:
			LCD_Str("state=WARNING ");
			break;
		case FCW_DANGER:
			LCD_Str("state=DANGER  ");
			break;
		case FCW_ERROR:
			LCD_Str("state=ERROR   ");
			break;
		default:
			LCD_Str("state=ERROR   ");
			break;
	}
}
void buzzer_player(uint8_t state)
{
	switch(state)
	{
		case FCW_SAFE:
		buz_enable = 0;
		buz_pattern = 0;
		PORTG &= ~_BV(BUZZ_PIN_BIT);
		break;

		case FCW_WARNING:
		buz_enable = 1;
		buz_pattern = 1;  // 더블 비프
		buz_half_ticks = buz_calc_half_ticks(700*4); // 낮은 주파수

		buz_phase_cnt = 0;
		warn_step = 0;
		break;

		case FCW_DANGER:
		buz_enable = 1;
		buz_pattern = 0;  // 연속음
		buz_half_ticks = buz_calc_half_ticks(2200*4); // 높은 주파수
		break;

		case FCW_ERROR:
		buz_enable = 1;
		buz_pattern = 1;
		buz_half_ticks = buz_calc_half_ticks(2500*10);
		buz_phase_cnt = 0;
		warn_step = 0;
		break;
	}
}


/*===============MAIN_MCU와 송수신==========*/
//uart0 데이터 전송 인터럽트
ISR(USART0_UDRE_vect){
	//하위 부터전송
	UDR0=srf_buf[srf_buf_idx++];
	
	if(srf_buf_idx>=2){
		UCSR0B &= ~(1<<UDRIE0);
		srf_buf_idx=0;
	}
	
	
}

//uart0 데이터 수신 인터럽트
ISR(USART0_RX_vect){
	rx_buf[rx_idx++]=UDR0;
	if(rx_idx>=2){
		speedflag_buf=rx_buf[0];
		fcw_state_buf=rx_buf[1];	
		rx_complete_flag=true;
		rx_idx=0;
	}
	
}

// 타이머카운터0 isr
ISR(TIMER0_COMP_vect){
	ti_Cnt_1ms++;
	if(ti_Cnt_1ms>=100){
		measure_ready=1;
		ti_Cnt_1ms=0;
	}
}
ISR(TIMER2_COMP_vect)
{
	if (!buz_enable || buz_half_ticks == 0) {
		buz_toggle_cnt = 0;
		return;
	}

	/* ===== WARNING : double beep FSM ===== */
	if (buz_pattern && fcw_state_buf == FCW_WARNING)
	{
		buz_phase_cnt++;

		switch (warn_step)
		{
			case 0: // ON1 (80ms)
			if (buz_phase_cnt >= MS_TO_TICKS(80)) {
				warn_step = 1;
				buz_phase_cnt = 0;
				PORTG &= ~_BV(BUZZ_PIN_BIT);
				buz_toggle_cnt = 0;
			}
			break;

			case 1: // OFF1 (120ms)
			if (buz_phase_cnt >= MS_TO_TICKS(120)) {
				warn_step = 2;
				buz_phase_cnt = 0;
			}
			return;

			case 2: // ON2 (80ms)
			if (buz_phase_cnt >= MS_TO_TICKS(80)) {
				warn_step = 3;
				buz_phase_cnt = 0;
				PORTG &= ~_BV(BUZZ_PIN_BIT);
				buz_toggle_cnt = 0;
			}
			break;

			case 3: // OFF_LONG (600ms)
			if (buz_phase_cnt >= MS_TO_TICKS(600)) {
				warn_step = 0;
				buz_phase_cnt = 0;
			}
			return;
		}
	}

	/* ===== 주파수 토글 (WARNING ON 구간 + DANGER 연속) ===== */
	if (++buz_toggle_cnt >= buz_half_ticks) {
		buz_toggle_cnt = 0;
		PORTG ^= _BV(BUZZ_PIN_BIT);
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
	usart0_init();
	LCD_Init();
	Timer0_Init();
	Init_TWI();	
	pwm1_init();
	pwm3_init();
	Timer2_Init();
	motor_speed_set(speed);

	
	/* PWM 핀 */
	DDRB |= (1<<PB5) | (1<<PB6) | (1<<PB0) | (1<<PB1) | (1<<PB2) | (1<<PB3);
	DDRE |= (1<<PE3) | (1<<PE4);
	DDRD |= (1<<PD4) | (1<<PD5) | (1<<PD6) | (1<<PD7);
	DDRG |= _BV(BUZZ_PIN_BIT);
	PORTG &= ~_BV(BUZZ_PIN_BIT);
	char Sonar_Addr=SRF02_ADDR_0;
	unsigned int Sonar_range;
	char Message[40];
	unsigned int res=0;
	uint8_t motor_flag=0;
	uint8_t motor_dir_flag=FORWARD;
	uint8_t fcw_state=0;



		
	uint8_t ret = SRF02_i2C_Write(0xE0, 0, 0x51);
	LCD_Pos(1,0);
	sprintf(Message, "W:%d", ret);
	LCD_Str(Message);
	
	startRanging(Sonar_Addr);
	ti_Cnt_1ms=0;
	
	
	sei();
	motor_driection(BACKWARD);
	motor_drive(MOTOR_SPEED_basic, &speed);
	while (1)
	{
		
		if(rx_complete_flag){
			main_rx_cmd_uart0(&motor_flag, &fcw_state);
			buzzer_player(fcw_state);		
			rx_complete_flag=false;
		}
		
		motor_driection(BACKWARD);
		motor_drive(motor_flag,&speed);
		
		if(measure_ready==1){
			measure_ready=0;
			
			
			res = getRange(Sonar_Addr, &Sonar_range);
			
			if(res !=0){
				LCD_Pos(0,0);
				LCD_Str("Dist=");
				LCD_Pos(1,5);
				LCD_Str("ERR           ");
				continue;
			}
			Ultrasonic_filtered(&Sonar_range);
			//main에 현재 거리값 전송
			send_ultra_to_sub_uart0(&Sonar_range);
			
			//sub lcd 출력
			LCD_Pos(0,0);
			sprintf(Message,"Dist= %03u cm   ", (unsigned int)Sonar_range);
			LCD_Str(Message);
			fcw_state_to_string(fcw_state);
		
			//거리 재측정
			startRanging(Sonar_Addr);
		}
		
	
	}
	
		
}
	
	
	

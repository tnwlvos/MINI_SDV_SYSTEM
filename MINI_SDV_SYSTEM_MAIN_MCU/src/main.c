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
#include <stdio.h>
#include <util/delay.h>
#include "system_state.h"
#include "hal_uart.h"
#include "lcd_gcc.h"
#include "sub_link.h"
#include "pc_link.h"
#include "control_logic.h"
#include "ota_bridge.h"
#include "fcw_logic.h"
#include "hal_lcd.h"
#include "parameter.h"
//================파라미터 설정===================

#define LCD_UPDATE_CNT 10

static uint16_t lcd_cnt = 0;

//=================함수 초기화===============

void disable_jtag()
{
	MCUCSR |= (1<<JTD);
	MCUCSR |= (1<<JTD);
}



void OTA_Bridge_Process(void) {}
int main(void)
{
	disable_jtag();
	SystemState_Init();
	Parameter_Init();
	LCD_Init();
	SUB_Init();
	PC_Init();
	OTA_Bridge_Init();
	
	sei();
	char v_msg[32];
	char msg[32];
	static uint8_t last_fcw_state=FCW_SAFE;
	PC_ProcessTx();
	_delay_ms(500);
	uint16_t ttc10;
	
	while (1)
	{
		
		PC_ProcessRx();   
		if (sdv_sys.ota_active) {	
			OTA_Bridge_Process();
			_delay_ms(200);
			continue;
		}
		else if(!sdv_sys.ota_active)
		{
			
			if(sdv_sys.distance_flag){
				//if(sdv_sys.mode==MODE_EMERGENCY && sdv_sys.distance_cm>=100){
					//Control_ClearEmergency();
				//} 
				
				corrected_distance();
				fcw_update();
				sdv_sys.distance_flag=false;
				Control_UpdateFromFCW();
				//sub로 모터 명령 전송
				SUB_TX_motorcmd();
				
				//lcd 출력===============================================
				ttc10=sdv_sys.ttc*10;
				//상태가 바뀌면 바로 출력
				if(sdv_sys.fcw_state != last_fcw_state)
				{
					lcd_cnt=0;
					LCD_Pos(0,0);
					sprintf(v_msg, "Speed: %d cm/s   ",(unsigned int)sdv_sys.speed_cms);
					LCD_Str(v_msg);
					LCD_Pos(1,0);
					sprintf(msg, "TTC:%2u.%1u s   ",(unsigned)(ttc10/10), (unsigned)(ttc10%10));
					LCD_Str(msg);
					
				}
				//상태 유지시 1초에 한번 출력
				else if(lcd_cnt<LCD_UPDATE_CNT){
					lcd_cnt++;
					
				}
				else{
					lcd_cnt=0;
					LCD_Pos(0,0);
					sprintf(v_msg, "Speed: %d cm/s   ",(unsigned int)sdv_sys.speed_cms);
					LCD_Str(v_msg);
					LCD_Pos(1,0);
					sprintf(msg, "TTC:%2u.%1u s   ",(unsigned)(ttc10/10), (unsigned)(ttc10%10));
					LCD_Str(msg);
				}
				//==============================================================================
				PC_ProcessTx();
				
			
			}		
		}
		else{
			OTA_Bridge_Process();
		}
		
	}
}

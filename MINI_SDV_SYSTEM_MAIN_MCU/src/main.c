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

#include "system_state.h"
#include "hal_uart.h"
#include "lcd_gcc.h"
#include "sub_link.h"
#include "pc_link.h"
#include "control_logic.h"
#include "ota_bridge.h"
//================파라미터 설정===================



//=================함수 초기화===============

void disable_jtag()
{
	MCUCSR |= (1<<JTD);
	MCUCSR |= (1<<JTD);
}


int main(void)
{
	disable_jtag();
	SystemState_Init();
	LCD_Init();
	SUB_Init();
	PC_Init();
	OTA_Bridge_Init();
	
	sei();
	
	
	char msg[32];
	PC_ProcessTx();
	_delay_ms(500);
	while (1)
	{
		if(!sdv_sys.ota_active)
		{
			if(sdv_sys.distance_flag){
				//if(sdv_sys.mode==MODE_EMERGENCY && sdv_sys.distance_cm>=100){
					//Control_ClearEmergency();
				//} 
				fcw_update();
				sdv_sys.distance_flag=false;
				Control_UpdateFromDistance();
				//sub로 모터 명령 전송
				SUB_TX_motorcmd();
				LCD_Pos(0,0);
				sprintf(msg,"Dist=%3d cm",(uint16_t)sdv_sys.distance_cm);
				LCD_Str(msg);
				LCD_Pos(1,0);
				sprintf(msg,"ttc=%4d s",(uint16_t)sdv_sys.ttc);
				LCD_Str(msg);
				
				PC_ProcessTx();
				
			
			}		
		}
		else{
			OTA_Bridge_Process();
		}
		
	}
}

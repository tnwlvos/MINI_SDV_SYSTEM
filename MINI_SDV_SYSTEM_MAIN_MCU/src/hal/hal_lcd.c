///*
 //* hal_lcd.c
 //*
 //* Created: 2025-12-09 오전 10:56:22
 //*  Author: LEESANGHUN
 //*/ 
//#include <avr/io.h>
//#include <avr/interrupt.h>
//#include <stdio.h>
//#include "hal_lcd.h"
//#include "system_state.h"
//#include "lcd_gcc.h"
//#include "fcw_logic.h"
//#define LCD_UPDATE_CNT 10
//static float ttc_disp = 0.0f;
//static uint16_t lcd_cnt = 0;
//
//void LCD_Update(){
	//char msg[32];
	//static uint8_t last_fcw_state;
	//if(sdv_sys.fcw_state != last_fcw_state)
	//{
		//lcd_cnt=0;
		//fcw_state_to_string(sdv_sys.fcw_state);
//
		//LCD_Pos(1,0);
		//sprintf(msg, "TTC:%d  ",(uint16_t)sdv_sys.ttc);
		//LCD_Str(msg);
		//return;
	//}
	//if(lcd_cnt<LCD_UPDATE_CNT){
		//lcd_cnt++;
		//return;
	//}
	//else{
		//lcd_cnt=0;
		//fcw_state_to_string(sdv_sys.fcw_state);
//
		//LCD_Pos(1,0);
		//sprintf(msg, "TTC:%d  ",(uint16_t)sdv_sys.ttc);
		//LCD_Str(msg);
	//}
//}
//
//
//

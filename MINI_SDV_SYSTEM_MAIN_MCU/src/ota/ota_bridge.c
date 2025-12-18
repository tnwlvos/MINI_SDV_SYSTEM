/*
 * ota_bridge.c
 *
 * Created: 2025-12-09 오전 10:57:40
 *  Author: LEESANGHUN
 */ 

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/eeprom.h>
#include <avr/wdt.h>

#include <avr/interrupt.h>
#include <string.h>
#include <stdio.h>
#include "parameter.h"
#include "ota_bridge.h"
#include "system_state.h"
#include "pc_link.h"
#include "sub_link.h"
// EEPROM에 OTA 요청 저장 (부트로더가 읽음)
#define EE_OTA_REQ_ADDR ((uint8_t*)0)
#define OTA_REQ_MAGIC  0xA5



void OTA_Bridge_Init(void)
{
	sdv_sys.ota_active = false;
	sdv_sys.ota_target = OTA_IDLE;
}

void OTA_Bridge_Begin(OtaTarget target){
	sdv_sys.ota_active = true;
	sdv_sys.ota_target = target;
	
	if(target == OTA_TARGET_MAIN){
		eeprom_update_byte(EE_OTA_REQ_ADDR, OTA_REQ_MAGIC);
		// 2) ACK 찍고
		PC_SendLine("OTA:ACK:BEGIN:MAIN");
		sdv_sys.motor_cmd = MOTOR_STOP;
		SUB_TX_motorcmd();

		// 2) STOP 2바이트 전송이 끝날 때까지 잠깐 대기(권장)
		while (UCSR0B & (1<<UDRIE0)) { }   // UDRE TX interrupt 켜져있으면 아직 전송중

		// 3) 그 다음에 서브 링크 suspend
		sdv_sys.sub_link_suspended = true;
		Send_parameter();//PC에게 사용자가 변환할 수 있는 현재 파라미터 값 제공
		
	}
	else if (target == OTA_TARGET_SUB){
		PC_SendLine("OTA:ACK:BEGIN:SUB");
		SUB_SendToken2(0xFF, 0xFF);
		
	}
}
void OTA_Bridge_Data(const char *line)
{
	char key[32];
	int  val;
	float fval;
	if(!sdv_sys.ota_active) return;
	
	if (sdv_sys.ota_target == OTA_TARGET_SUB) {
		// payload(인텔헥스 한 줄)를 SUB로 넘김
		SUB_SendLine(line);
		PC_SendLine("OTA:ACK:DATA:SUB");
		} 
	else if (sdv_sys.ota_target == OTA_TARGET_MAIN) {
		int r=sscanf(line,":PARAM:%31[^:]:%d",key,&val);
		if (r != 2) {
			PC_SendLine("OTA:NAK:PARAM_FORMAT");  // 바로 보이게
			return;
		}
		if (strcmp(key, "TTC_DANGER") == 0 || strcmp(key, "TTC_WARNING") == 0) {
			fval = val / 10.0f;
		}
		else
			fval=(float)val;
		Parameter_Update(key,fval);
		
		
	}
}	
void OTA_Bridge_End(void)
{
	
	if (sdv_sys.ota_target == OTA_TARGET_MAIN) {
		 sdv_sys.sub_link_suspended = false;  
	 }
	if(sdv_sys.ota_target== OTA_TARGET_SUB){
		SUB_SendToken2(0xFA, 0xFA);
	}
	sdv_sys.ota_active = false;
	 sdv_sys.ota_target = OTA_IDLE;
	 sub_proto_mode = SUB_PROTO_BINARY;

	 // ★ 여기: STAY면 UP으로 강제 킥
	 uint8_t cmd = sdv_sys.motor_cmd;
	 if (cmd == SPEED_STAY) cmd = SPEED_UP;

	 SUB_SendMotorCmdNow(cmd, sdv_sys.fcw_state);

	Parameter_SaveIfChange();   //  변경된 경우에만 EEPROM에 저장

	PC_SendLine("OTA:ACK:END");
}
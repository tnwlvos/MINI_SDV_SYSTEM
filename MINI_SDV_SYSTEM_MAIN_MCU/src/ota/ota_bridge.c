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
		Send_parameter();//PC에게 사용자가 변환할 수 있는 현재 파라미터 값 제공
		
	}
	else if (target == OTA_TARGET_SUB){
		PC_SendLine("OTA:ACK:BEGIN:SUB");
		SUB_SendToken2(0xFF, 0xFF);
		sub_proto_mode = SUB_PROTO_OTA_TEXT;
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
		sscanf(line,":PARAM:%31[^:]:%d",key,&val);
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
	sdv_sys.ota_active = false;
	sdv_sys.ota_target = OTA_IDLE;
	sub_proto_mode = SUB_PROTO_BINARY;

	Parameter_SaveIfChange();   // ✅ 변경된 경우에만 EEPROM에 저장

	PC_SendLine("OTA:ACK:END");
}
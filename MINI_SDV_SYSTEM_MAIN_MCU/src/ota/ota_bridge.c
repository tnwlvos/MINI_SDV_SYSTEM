/*
 * ota_bridge.c
 *
 * Created: 2025-12-09 오전 10:57:40
 *  Author: LEESANGHUN
 */ 
#include "ota_bridge.h"
#include "system_state.h"
#include "pc_link.h"
#include "sub_link.h"

void OTA_Bridge_Init(void)
{
	sdv_sys.ota_active = false;
	sdv_sys.ota_target = OTA_IDLE;
}

void OTA_Bridge_Begin(OtaTarget target){
	sdv_sys.ota_active = true;
	sdv_sys.ota_target = target;
	
	if(target == OTA_TARGET_MAIN)
		PC_SendLine("OTA:ACK:BEGIN:MAIN");
	
	else if (target == OTA_TARGET_SUB){
		PC_SendLine("OTA:ACK:BEGIN:SUB");
		SUB_SendToken2(0xFF, 0xFF);
		sub_proto_mode = SUB_PROTO_OTA_TEXT;
	}
}
void OTA_Bridge_Data(const char *line)
{
	
	if(!sdv_sys.ota_active) return;
	
	if (sdv_sys.ota_target == OTA_TARGET_SUB) {
		// payload(인텔헥스 한 줄)를 SUB로 넘김
		SUB_SendLine(line);
		PC_SendLine("OTA:ACK:DATA:SUB");
		} 
	else if (sdv_sys.ota_target == OTA_TARGET_MAIN) {
		// TODO: MAIN 플래시 기록 큐에 저장 (다음 단계)
		PC_SendLine("OTA:ACK:DATA:MAIN");
	}
}	
void OTA_Bridge_End(void)
{
	sdv_sys.ota_active = false;
	sdv_sys.ota_target = OTA_IDLE;
	sub_proto_mode = SUB_PROTO_BINARY;
	PC_SendLine("OTA:ACK:END");
	
}
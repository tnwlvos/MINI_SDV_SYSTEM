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
#include <util/delay.h>
#include <avr/interrupt.h>
#include <string.h>

#include "ota_bridge.h"
#include "system_state.h"
#include "pc_link.h"
#include "sub_link.h"
// EEPROM에 OTA 요청 저장 (부트로더가 읽음)
#define EE_OTA_REQ_ADDR ((uint8_t*)0)
#define OTA_REQ_MAGIC  0xA5

static void system_reset_to_bootloader(void)
{
	cli();
	wdt_enable(WDTO_15MS);
	while (1) { }
}

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

		// 3) 잠깐 대기(PC가 ACK 받을 시간)
		_delay_ms(50);

		// 4) 리셋 -> BOOTRST=ON이면 부트로더로 진입
		wdt_enable(WDTO_15MS);
		while(1);
	}
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
		
	}
}	
void OTA_Bridge_End(void)
{
	sdv_sys.ota_active = false;
	sdv_sys.ota_target = OTA_IDLE;
	sub_proto_mode = SUB_PROTO_BINARY;
	PC_SendLine("OTA:ACK:END");
	
}
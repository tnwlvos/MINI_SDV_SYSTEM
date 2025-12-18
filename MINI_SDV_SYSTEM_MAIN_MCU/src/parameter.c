/*
 * parameter.c
 *
 * Created: 2025-12-18 오전 11:56:01
 *  Author: LEESANGHUN
 */ 

#include "parameter.h"
#include <stdint.h>
#include "pc_link.h"
#include <stdio.h> 
#include <avr/eeprom.h>
#include <util/atomic.h>


// ================================
// EEPROM 메모리 맵 
// 0x00 : OTA 요청 플래그 (너 ota_bridge.c에서 이미 사용중)
// 0x01 : PARAM magic
// 0x02 : PARAM version
// 0x03~0x04 : CRC16 (2 bytes)
// 0x05~ : Parameter struct 저장
// ================================


#define EE_PARAM_MAGIC_ADDR      ((uint8_t*)0x01)
#define EE_PARAM_VER_ADDR		 ((uint8_t*)0x02)
#define EE_PARAM_DATA_ADDR       ((uint8_t*)0x03)

#define EE_PARAM_MAGIC           (0x5A)
#define EE_PARAM_VER	         (0x01)

Parameter parameter;
volatile uint8_t parameter_change = 0;
void Parameter_SetDefault(){
	parameter.ttc_danger=1.5;
	parameter.ttc_warning=5.0;
	parameter.D_caution=30;
	parameter.D_Emergency=15;
	parameter_change = 0;

}
void Parameter_Init(void)
{
	uint8_t magic = eeprom_read_byte(EE_PARAM_MAGIC_ADDR);
	uint8_t ver   = eeprom_read_byte(EE_PARAM_VER_ADDR);

	if (magic != EE_PARAM_MAGIC || ver != EE_PARAM_VER) {
		// EEPROM이 비었거나(초기) 버전이 다르면 기본값
		Parameter_SetDefault();
		return;
	}

	// 검증 통과 -> EEPROM에서 구조체 통째 로드
	eeprom_read_block(&parameter, EE_PARAM_DATA_ADDR, sizeof(Parameter));
	parameter_change = 0;
}

void Parameter_SaveNow(void)
{
	// 저장 중 값 흔들림 방지용 스냅샷
	Parameter snap;
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
		snap = parameter;
	}

	// update_ 계열: 값이 동일하면 실제 write 안 해서 EEPROM 수명에 유리
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
		eeprom_update_byte(EE_PARAM_MAGIC_ADDR, EE_PARAM_MAGIC);
		eeprom_update_byte(EE_PARAM_VER_ADDR, EE_PARAM_VER);
		eeprom_update_block(&snap, EE_PARAM_DATA_ADDR, sizeof(Parameter));
		parameter_change = 0;
	}
}

void Parameter_SaveIfChange(void)
{
	if (!parameter_change) return;
	Parameter_SaveNow();
}


void Send_parameter(){
	char para_buf[128];

	int ttc_d_x10 = (int)(parameter.ttc_danger  * 10.0f);
	int ttc_w_x10 = (int)(parameter.ttc_warning * 10.0f);

	int ttc_d_int  = ttc_d_x10 / 10;
	int ttc_d_frac = ttc_d_x10 % 10;

	int ttc_w_int  = ttc_w_x10 / 10;
	int ttc_w_frac = ttc_w_x10 % 10;

	snprintf(para_buf, sizeof(para_buf),
	"TTC_DANGER=%d.%d;TTC_WARNING=%d.%d;D_CAUTION=%u;D_EMERGENCY=%u;",
	ttc_d_int, ttc_d_frac,
	ttc_w_int, ttc_w_frac,
	(unsigned)parameter.D_caution,
	(unsigned)parameter.D_Emergency);

	PC_SendLine(para_buf);
}
void Parameter_Update(const char* key, float val){
	
	if (strcmp(key, "TTC_DANGER") == 0)
	{
		if (val > 0.1f && val < 10.0f)
			parameter.ttc_danger = val;
		parameter_change=1;
		PC_SendLine("OTA:ACK:PARAM");
		Send_parameter();
		return;
	}
	else if (strcmp(key, "TTC_WARNING") == 0)
	{
		if (val > 0.1f && val < 15.0f)
			parameter.ttc_warning = val;
		parameter_change=1;
		PC_SendLine("OTA:ACK:PARAM");
		Send_parameter();
		return;
	}
	else if (strcmp(key, "D_CAUTION") == 0)
	{
		if (val >= 0 && val <= 500)
			parameter.D_caution = (uint16_t)val;
		parameter_change=1;
		PC_SendLine("OTA:ACK:PARAM");
		Send_parameter();
		return;
	}
	else if (strcmp(key, "D_EMERGENCY") == 0)
	{
		if (val >= 0 && val <= 500)
			parameter.D_Emergency = (uint16_t)val;
		parameter_change=1;
		PC_SendLine("OTA:ACK:PARAM");
		Send_parameter();
		return;
	}
	PC_SendLine("OTA:NACK:PARAM");
	
}


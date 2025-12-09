/*
 * ota_bridge.c
 *
 * Created: 2025-12-09 오전 10:57:40
 *  Author: LEESANGHUN
 */ 
#include "ota_bridge.h"
#include "system_state.h"

void OTA_Bridge_Init(){}

void OTA_Bridge_Start(){
	sdv_sys.ota_active=true;
	
}
void OTA_Bridge_Stop(){
	sdv_sys.ota_active=false;
	
}
void OTA_Bridge_Process(void)
{
	// TODO:
	//  PC UART0에서 읽은 바이트 → SUB UART1로 그대로 전달
	//  SUB UART1에서 읽은 바이트 → PC UART0로 전달
	//  → 브릿지 역할
}
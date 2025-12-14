/*
 * system_state.c
 *
 * Created: 2025-12-09 오전 10:55:40
 *  Author: LEESANGHUN
 */ 
#include "system_state.h"

SystemState sdv_sys;
volatile sub_proto_t sub_proto_mode = SUB_PROTO_BINARY;  

void SystemState_Init(void)
{
	sdv_sys.mode=MODE_AUTO;
	sdv_sys.motor_cmd=SPEED_STAY;
	sdv_sys.last_motor_cmd=SPEED_STAY;
	sdv_sys.motor_dir=FORWARD;
	sdv_sys.speed_cms=0.0;
	sdv_sys.last_speed_cms=0;
	sdv_sys.fcw_state=FCW_SAFE;
	sdv_sys.distance_cm=0.0;
	sdv_sys.ttc= -1.0;
	sdv_sys.last_distance_cm=0.0;
	sdv_sys.distance_flag=false;
	sdv_sys.pc_connect=false;
	sdv_sys.ota_active=false;
	sdv_sys.ota_target = OTA_IDLE;
	
}
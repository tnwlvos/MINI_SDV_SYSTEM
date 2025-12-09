/*
 * control_logic.c
 *
 * Created: 2025-12-09 오전 10:57:20
 *  Author: LEESANGHUN
 */ 
#include "control_logic.h"

#define D_CAUTION   50   // cm
#define D_EMERGENCY 20   // cm

void Control_Init()
{
	
}

void Control_UpdateFromDistance(void)
{
	uint16_t d= sdv_sys.distance_cm;
	
	if(d<=D_EMERGENCY)
	{
		sdv_sys.motor_cmd=MOTOR_STOP;
		sdv_sys.mode=MODE_EMERGENCY;
	}
	else if(d<=D_CAUTION)
	{
		sdv_sys.motor_cmd=SPEED_DOWN;
		if(sdv_sys.mode != MODE_EMERGENCY)
			sdv_sys.mode=MODE_AUTO;
	}
	else
	{
		if(sdv_sys.mode != MODE_EMERGENCY)
		{
			if(sdv_sys.last_motor_cmd == SPEED_DOWN)
				sdv_sys.motor_cmd=SPEED_UP;
			else if(sdv_sys.last_motor_cmd == MOTOR_STOP)
				sdv_sys.motor_cmd=SPEED_DOWN;
			else
				sdv_sys.motor_cmd=SPEED_STAY;
			sdv_sys.mode=MODE_AUTO;
		}
		else
		{
			sdv_sys.motor_cmd=MOTOR_STOP;
		}
		
	}
	
}
void Control_ClearEmergency(void)
{
	sdv_sys.mode      = MODE_AUTO;
	sdv_sys.motor_cmd = MOTOR_STOP; // 처음엔 정지 상태로 시작
}
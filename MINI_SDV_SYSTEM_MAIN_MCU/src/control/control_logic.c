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
	fcw_states state= sdv_sys.fcw_state;
	
	if(state==FCW_DANGER)
	{
		sdv_sys.motor_cmd=MOTOR_STOP;
		//sdv_sys.mode=MODE_EMERGENCY;
	}
	else if(state==FCW_WARNING)
	{
		sdv_sys.motor_cmd=SPEED_DOWN;
		//if(sdv_sys.mode != MODE_EMERGENCY)
			//sdv_sys.mode=MODE_AUTO;
	}
	else if(state== FCW_SAFE)
	{
		//if(sdv_sys.mode != MODE_EMERGENCY)
		//{
			//if(sdv_sys.last_motor_cmd == SPEED_DOWN)
				//sdv_sys.motor_cmd=SPEED_UP;
			//else if(sdv_sys.last_motor_cmd == MOTOR_STOP)
				//sdv_sys.motor_cmd=SPEED_DOWN;
			//else
				//sdv_sys.motor_cmd=SPEED_STAY;
			//sdv_sys.mode=MODE_AUTO;
		//}
		//else
		//{
			//sdv_sys.motor_cmd=MOTOR_STOP;
		//}
		if(sdv_sys.last_motor_cmd==MOTOR_STOP){
			sdv_sys.motor_cmd=SPEED_DOWN;
			
		}
		else if(sdv_sys.last_motor_cmd==SPEED_DOWN){
			sdv_sys.motor_cmd=SPEED_UP;
		}
		else
			{
				sdv_sys.motor_cmd=SPEED_STAY;
			}
		
	}
	 else if(state == FCW_ERROR)
	 {
		 sdv_sys.motor_cmd = MOTOR_STOP;  // 안전 기본값
	 }
	
}
void Control_ClearEmergency(void)
{
	sdv_sys.mode      = MODE_AUTO;
	sdv_sys.motor_cmd = MOTOR_STOP; // 처음엔 정지 상태로 시작
}
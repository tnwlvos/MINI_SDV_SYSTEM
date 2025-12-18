/*
 * control_logic.c
 *
 * Created: 2025-12-09 오전 10:57:20
 *  Author: LEESANGHUN
 */ 
#include "control_logic.h"
#include "parameter.h"


void Control_Init()
{
	
}

void Control_UpdateFromDistance(void)
{
	uint16_t d= sdv_sys.distance_cm;
	
	if(d<=parameter.D_Emergency)
	{
		sdv_sys.motor_cmd=MOTOR_STOP;		
	}
	else if(d<=parameter.D_caution)
	{
		sdv_sys.motor_cmd=SPEED_DOWN;
	}
	else
	{
		
		if(sdv_sys.last_motor_cmd == SPEED_DOWN)
			sdv_sys.motor_cmd=SPEED_UP;
		else if(sdv_sys.last_motor_cmd == MOTOR_STOP)
			sdv_sys.motor_cmd=SPEED_DOWN;
		else
			sdv_sys.motor_cmd=SPEED_STAY;
		
		}
	
		
	
	
}
void Control_UpdateFromFCW(void)
{
	fcw_states state= sdv_sys.fcw_state;
	static unsigned int safe_cnt=0;
	static uint8_t restarted = 0; // 재출발 완료 플래그
	if(state != FCW_SAFE){
		safe_cnt = 0;
		restarted = 0;
	}

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
		if(!restarted){
			if(safe_cnt < 15){
				safe_cnt++;
				sdv_sys.motor_cmd = MOTOR_STOP; // 기다리는 동안 정지 유지
			}
			else{
				Control_UpdateFromDistance();  // 재출발 1회
				restarted = 1;
			}
		}
		else{
			Control_UpdateFromDistance();      // 이후는 거리 기반 주행
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
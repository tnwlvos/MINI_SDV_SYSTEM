/*
 * fcw_logic.c
 *
 * Created: 2025-12-11 오후 11:36:58
 *  Author: LEESANGHUN
 */
#include "system_state.h"
#include "fcw_logic.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include "lcd_gcc.h"
#define DANGER_RELEASE_CNT 8
#define SENSOR_OFFSET_CM 11.5f

void corrected_distance()
{
	float d = sdv_sys.distance_cm - SENSOR_OFFSET_CM;

	if (d < 0.0f)
		d = 0.0f;
	sdv_sys.distance_cm=d;
	
}


void fcw_update(void){
	
	static uint32_t danger_cnt = 0;
	
	float ttc = fcw_get_ttc();
	sdv_sys.ttc=ttc;
	if(sdv_sys.fcw_state==FCW_DANGER){
		if(danger_cnt > 0){			
			danger_cnt--;
			return;
		}
		//위험 상태에서 탈출 조건의 ttc는 좀더 여유를 줌
		 if(ttc < 1.2f && ttc > 0){
			   return;
		   }
		
	}
	if (ttc<1.5 &&  ttc>0)
	{
		sdv_sys.fcw_state=FCW_DANGER;
		danger_cnt=DANGER_RELEASE_CNT;
		return;
	}
	if(ttc<5.0){
		sdv_sys.fcw_state=FCW_WARNING;
	}
	else
		sdv_sys.fcw_state=FCW_SAFE;
	sdv_sys.ttc=ttc;
	
	
}


const char* fcw_state_to_string(void){
	switch(sdv_sys.fcw_state){
		case FCW_SAFE:
		return "SAFE";
		case FCW_WARNING:
		return "WARNING";
		case FCW_DANGER:
		return "DANGER";
		case FCW_ERROR:
		return "ERROR";
		default:
		return "UNKNOWN";
	}
}




void fcw_get_relative_speed(void){
	
	// NOTE: distance update period is fixed at 100ms from SUB MCU
	float dt = 0.1;  // seconds
	float delta_distance = sdv_sys.distance_cm - sdv_sys.last_distance_cm;
	float v_cms = delta_distance / dt; // cm/s
	const float beta = 0.4; // 필터 계수 (0 < beta < 1)
	sdv_sys.speed_cms= beta * v_cms + (1 - beta) *sdv_sys.last_speed_cms;
	sdv_sys.last_speed_cms = sdv_sys.speed_cms;
	sdv_sys.last_distance_cm = sdv_sys.distance_cm;
	

}

float fcw_get_ttc(void){
	fcw_get_relative_speed();
	// sdv_sys.speed_cms = (float)relative_speed; // Convert cm/ss
	int v_min=1.0;
	if (sdv_sys.speed_cms >= -v_min){
		return 9999.0; // Object is moving away
	}
	float ttc = sdv_sys.distance_cm / (-sdv_sys.speed_cms);
	
	return ttc;
}
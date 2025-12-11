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



void fcw_update(void){
	float ttc = fcw_get_ttc();
	if(ttc < 0.0){
		sdv_sys.fcw_state = FCW_ERROR;
	}
	else if(ttc < 1.0 && ttc >= 0.0){
		sdv_sys.fcw_state = FCW_DANGER;
	}
	
	else if(ttc >= 1.0 && ttc < 3.0){
		sdv_sys.fcw_state = FCW_WARNING;
	}
	else{
		sdv_sys.fcw_state = FCW_SAFE;
	}
	
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

int16_t fcw_get_relative_speed(void){
	int16_t delta_distance = sdv_sys.last_distance_cm - sdv_sys.distance_cm;
	int16_t abs_delta_distance = delta_distance >=0 ? delta_distance : -delta_distance;
	return abs_delta_distance*10; // cm/s

}

float fcw_get_ttc(void){
	int16_t relative_speed = fcw_get_relative_speed();
	sdv_sys.speed_cms = (float)relative_speed; // Convert cm/ss
	if(relative_speed == 0){
		return 9999.0; // Infinite TTC
	}
	
	float ttc = (float)sdv_sys.distance_cm / relative_speed;
	return ttc;
}
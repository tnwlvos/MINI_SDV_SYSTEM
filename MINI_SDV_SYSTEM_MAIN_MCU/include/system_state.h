/*
 * system_state.h
 *
 * Created: 2025-12-09 오전 10:53:05
 *  Author: LEESANGHUN
 */ 


#ifndef SYSTEM_STATE_H_
#define SYSTEM_STATE_H_

#include <stdint.h>
#include <stdbool.h>

typedef enum {
	SUB_PROTO_BINARY = 0,
	SUB_PROTO_OTA_TEXT
} sub_proto_t;

extern volatile sub_proto_t sub_proto_mode;

typedef enum{
	MODE_AUTO = 0,
	MODE_MANUAL_BTN,
	MODE_EMERGENCY  
} ControlMode;


typedef enum {
	FCW_SAFE = 0,
	FCW_WARNING,
	FCW_DANGER,
	FCW_ERROR
} fcw_states;

typedef enum{
	SPEED_UP=1, 
	SPEED_DOWN ,
	MOTOR_STOP ,
	SPEED_STAY 
}MotorCmd;

typedef enum{
	FORWARD=0,
	BACKWARD
}MotorDir;

typedef enum {
	OTA_IDLE = 0,
	OTA_TARGET_MAIN,
	OTA_TARGET_SUB
} OtaTarget;



typedef struct{
	ControlMode mode;
	MotorCmd motor_cmd;
	MotorCmd last_motor_cmd;
	MotorDir motor_dir;
	float distance_cm;
	float last_distance_cm;
	float speed_cms;
	float last_speed_cms;
	float ttc;
	fcw_states fcw_state;
	bool distance_flag;	//새 거리값 들어왔는지 확인용 플래그
	bool pc_connect;	// PC 링크 감지용
	bool ota_active;	//ota 진행 중 여부

	OtaTarget ota_target; 

} SystemState;

extern SystemState sdv_sys;
	
void SystemState_Init(void);

#endif /* SYSTEM_STATE_H_ */
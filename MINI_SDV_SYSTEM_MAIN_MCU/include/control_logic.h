/*
 * control_logic.h
 *
 * Created: 2025-12-09 오전 10:54:53
 *  Author: LEESANGHUN
 */ 


#ifndef CONTROL_LOGIC_H_
#define CONTROL_LOGIC_H_
#include <stdint.h>
#include "system_state.h"

void Control_Init(void);
void Control_UpdateFromDistance(void);
void Control_UpdateFromFCW(void);
void Control_ClearEmergency(void);



#endif /* CONTROL_LOGIC_H_ */
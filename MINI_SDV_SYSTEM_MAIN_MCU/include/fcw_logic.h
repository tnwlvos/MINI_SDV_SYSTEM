/*
 * fcw_logic.h
 *
 * Created: 2025-12-11 오후 11:37:21
 *  Author: LEESANGHUN
 */ 


#ifndef FCW_LOGIC_H_
#define FCW_LOGIC_H_

#include <stdint.h>

void fcw_update(void);

const char* fcw_state_to_string(void);

void fcw_get_relative_speed(void);
float fcw_get_ttc(void);


#endif /* FCW_LOGIC_H_ */
/*
 * parameter.h
 *
 * Created: 2025-12-18 오전 11:56:22
 *  Author: LEESANGHUN
 */ 


#ifndef PARAMETER_H_
#define PARAMETER_H_
#include <stdint.h>

typedef struct __attribute__((packed)){
	float ttc_danger;
	float ttc_warning;
	uint16_t D_caution;
	uint16_t D_Emergency;
	

}Parameter;

extern volatile uint8_t parameter_change;
extern Parameter parameter;
void Parameter_Init();
void Parameter_SetDefault(void);      
void Send_parameter();
void Parameter_Update(const char* key, float val);
void Parameter_SaveNow(void);                    // EEPROM에 즉시 저장(실제 write)
void Parameter_SaveIfChange(void);

#endif /* PARAMETER_H_ */
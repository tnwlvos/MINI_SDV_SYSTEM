/*
 * ota_bridge.h
 *
 * Created: 2025-12-09 오전 10:55:12
 *  Author: LEESANGHUN
 */ 


#ifndef OTA_BRIDGE_H_
#define OTA_BRIDGE_H_
#include <stdint.h>
#include "system_state.h"
void OTA_Bridge_Init(void);
void OTA_Bridge_Begin(OtaTarget target);
void OTA_Bridge_Data(const char *line);
void OTA_Bridge_End(void);
void OTA_Bridge_Process(void); 
#endif /* OTA_BRIDGE_H_ */
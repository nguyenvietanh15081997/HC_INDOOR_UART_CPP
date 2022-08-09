/*
 * SwitchOnOff.h
 *
 *  Created on: Jan 24, 2022
 *      Author: anh
 */

#ifndef SWITCHONOFF_SWITCHONOFF_HPP_
#define SWITCHONOFF_SWITCHONOFF_HPP_

#include "../Include/Include.hpp"
#include "../logging/slog.h"

/*header for switch touch*/
#define SWITCH_4_CONTROL				(0x000b)
#define SWITCH_4_SCENE_SET				(0x010b)
#define SWITCH_4_SCENE_DEL 				(0x020b)
#define SWITCH_4_STATUS					(0x030b)
#define SWITCH_CONTROL_HSL				(0x050b)
#define SWITCH_CONTROL_COMBINE			(0x060b)
#define SWITCH_TIMER					(0x070b)

#define SWITCH_3_CONTROL				(0x000c)
#define SWITCH_3_SCENE_SET 				(0x010c)
#define SWITCH_3_SCENE_DEL 				(0x020c)
#define SWITCH_3_STATUS					(0x030c)

#define SWITCH_2_CONTROL				(0x000d)
#define SWITCH_2_SCENE_SET 				(0x010d)
#define SWITCH_2_SCENE_DEL 				(0x020d)
#define SWITCH_2_STATUS					(0x030d)

#define SWITCH_1_CONTROL				(0x000e)
#define SWITCH_1_SCENE_SET 				(0x010e)
#define SWITCH_1_SCENE_DEL 				(0x020e)
#define SWITCH_1_STATUS					(0x030e)

typedef enum{
	switch_enum_addscene,
	switch_enum_delscene,
	switch_enum_control,
	switch_enum_status,
	switch_enum_control_hsl,
	switch_enum_control_combine,
	switch_enum_timer
}switch_enum_cmd;

char IndexType (uint16_t typeSw);

char IndexOpcode (uint16_t opcode);

void Switch_Send_Uart(switch_enum_cmd typeCmd, uint16_t typeSw, uint16_t adr,
		uint8_t relayId, uint8_t relayValue, uint8_t relay1, uint8_t relay2,
		uint8_t relay3, uint8_t relay4, uint16_t sceneId, uint16_t h, uint16_t s, uint16_t l, uint32_t time) ;

void Rsp_Switch_Control(TS_GWIF_IncomingData * data);

void Rsp_Switch_Scene_Set(TS_GWIF_IncomingData * data);

void Rsp_Switch_Scene_Del(TS_GWIF_IncomingData * data);

void Rsp_Switch_Status(TS_GWIF_IncomingData * data);

void Rsp_Switch_RequestStatus(TS_GWIF_IncomingData * data);

void Rsp_Switch_ControlRGB (TS_GWIF_IncomingData *data);

void Rsp_Switch_Control_Combine (TS_GWIF_IncomingData * data);

void Rsp_Switch_Timer (TS_GWIF_IncomingData * data);

#endif /* SWITCHONOFF_SWITCHONOFF_HPP_ */


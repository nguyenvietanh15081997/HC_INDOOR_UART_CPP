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
#define SWITCH_4_CONTROL_HSL			(0x050b)
#define SWITCH_4_CONTROL_COMBINE		(0x060b)
#define SWITCH_4_TIMER					(0x070b)

#define REQUEST_STATUS_DEV_RGB          (0x090b)
#define UPDATE_STATUS_DEV_RGB			(0x080b)

#define SWITCH_3_CONTROL				(0x000c)
#define SWITCH_3_SCENE_SET 				(0x010c)
#define SWITCH_3_SCENE_DEL 				(0x020c)
#define SWITCH_3_STATUS					(0x030c)
#define SWITCH_3_CONTROL_HSL			(0x050c)
#define SWITCH_3_CONTROL_COMBINE		(0x060c)
#define SWITCH_3_TIMER					(0x070c)

#define SWITCH_2_CONTROL				(0x000d)
#define SWITCH_2_SCENE_SET 				(0x010d)
#define SWITCH_2_SCENE_DEL 				(0x020d)
#define SWITCH_2_STATUS					(0x030d)
#define SWITCH_2_CONTROL_HSL			(0x050d)
#define SWITCH_2_CONTROL_COMBINE		(0x060d)
#define SWITCH_2_TIMER					(0x070d)

#define SWITCH_1_CONTROL				(0x000e)
#define SWITCH_1_SCENE_SET 				(0x010e)
#define SWITCH_1_SCENE_DEL 				(0x020e)
#define SWITCH_1_STATUS					(0x030e)
#define SWITCH_1_CONTROL_HSL			(0x050e)
#define SWITCH_1_CONTROL_COMBINE		(0x060e)
#define SWITCH_1_TIMER					(0x070e)

#define SWITCH_1_TYPE					(22001)
#define SWITCH_2_TYPE					(22002)
#define SWITCH_3_TYPE					(22003)
#define SWITCH_4_TYPE					(22004)
#define BLN_TYPE						(22005)

#define SWITCH_RGB_1_TYPE				(22012)
#define SWITCH_RGB_2_TYPE				(22013)
#define SWITCH_RGB_3_TYPE				(22014)
#define SWITCH_RGB_4_TYPE				(22015)
#define	BLN_RGB_TYPE					(22016)
#define REM_RGB_TYPE					(22017)
#define CUA_CUON						(22018)

#define CONG_TAC_CO_1_TYPE				(24001)
#define CONG_TAC_CO_2_TYPE				(24002)
#define CONG_TAC_CO_3_TYPE				(24003)
#define CONG_TAC_CO_4_TYPE				(24004)
#define CONG_TAC_CO_BNL_TYPE			(24005)

#define BANG_CANH_RGB_AC				(23006)
#define CONTROL_HSL_BANG_CANH_AC		(0x0503)

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
		uint8_t relay3, uint8_t relay4, uint16_t sceneId, uint8_t r, uint8_t g, uint8_t b, uint32_t time);

void Rsp_Switch_Control(TS_GWIF_IncomingData * data);

void Rsp_Switch_Scene_Set(TS_GWIF_IncomingData * data);

void Rsp_Switch_Scene_Del(TS_GWIF_IncomingData * data);

void Rsp_Switch_Status(TS_GWIF_IncomingData * data);

void Rsp_Switch_RequestStatus(TS_GWIF_IncomingData * data);

void Rsp_RequestStatusRgb(TS_GWIF_IncomingData * data);

void Rsp_Switch_ControlRGB (TS_GWIF_IncomingData *data);

void Rsp_Switch_Control_Combine (TS_GWIF_IncomingData * data);

void Rsp_Switch_Timer (TS_GWIF_IncomingData * data);

#endif /* SWITCHONOFF_SWITCHONOFF_HPP_ */


/*
 * Curtain.h
 *
 *  Created on: Feb 21, 2022
 *      Author: anh
 */

#ifndef CURTAIN_CURTAIN_HPP_
#define CURTAIN_CURTAIN_HPP_

#include "../Include/Include.hpp"
#include "../Mqtt/Mqtt.hpp"
#include "../ProcessUart/OpCode.h"
#include "../logging/slog.h"

#define CURTAIN_CONTROL			0x0011
#define CURTAIN_STATUS_RSP		0x0311
#define CURTAIN_SCENE_SET		0x0111
#define CURTAIN_SCENE_DEL		0x0211
#define CURTAIN_CALIB			0x0411
#define CURTAIN_CONFIG_MOTOR 	0x0511

#define CURTAIN_OPEN			01
#define CURTAIN_CLOSE			00
#define CURTAIN_PAUSE			02
#define CURTAIN_OPEN_PERCENT	03

typedef enum{
	enum_curtain_control,
	enum_curtain_scene_set,
	enum_curtain_scene_del,
	enum_curtain_status_request,
	enum_curtain_calib,
	enum_curtain_config_motor
}curtain_cmd_t;

void CURTAIN_Cmd(curtain_cmd_t typeCmd, uint16_t adr, uint8_t typeControl, uint8_t percent, uint16_t idScene) ;

void CURTAIN_RSP_Control(TS_GWIF_IncomingData *data);

void CURTAIN_RSP_Scene_Set(TS_GWIF_IncomingData *data) ;

void CURTAIN_RSP_Scene_Del(TS_GWIF_IncomingData *data);

void CURTAIN_RSP_Status_Request(TS_GWIF_IncomingData *data) ;

void CURTAIN_RSP_ConfigMotor(TS_GWIF_IncomingData *data);

void CURTAIN_RSP_Calib(TS_GWIF_IncomingData *data);

void CURTAIN_RSP_PressBT( TS_GWIF_IncomingData * data);

#endif /* CURTAIN_CURTAIN_H_ */

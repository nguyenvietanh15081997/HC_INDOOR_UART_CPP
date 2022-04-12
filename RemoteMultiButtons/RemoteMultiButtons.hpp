/*
 * RemoteMultiButtons.h
 *
 *  Created on: Feb 21, 2022
 *      Author: anh
 */

#ifndef REMOTEMULTIBUTTONS_REMOTEMULTIBUTTONS_HPP_
#define REMOTEMULTIBUTTONS_REMOTEMULTIBUTTONS_HPP_

#include "../Include/Include.hpp"
#include "../Mqtt/Mqtt.hpp"
#include "../ProcessUart/OpCode.h"
#include "../logging/slog.h"

#define REMOTE_MUL_CONFIG_GROUP					0x0B0A
#define REMOTE_MUL_SCENE_SET					0x0102
#define REMOTE_MUL_SCENE_DEL					0x0202


/*Remote multi button rsp when press button*/
#define REMOTE_MUL_RSP_ONOFF_GROUP				0x0302
#define REMOTE_MUL_RSP_SCENE_ACTIVE_DEFAULT		0x0402
#define REMOTE_MUL_RSP_SCENE_ACTIVE				0x0502
#define REMOTE_MUL_RSP_UP_DOWN					0x0602
#define REMOTE_MUL_RSP_SWITCH_STATUS			0x0802
#define REMOTE_MUL_RSP_TIMER					0x0702

#define REMOTE_MUL_UP	   						1
#define REMOTE_MUL_DOWN							2
#define REMOTE_MUL_MODE_DIM						0
#define REMOTE_MUL_MODE_CCT						1
#define REMOTE_MUL_MODE_RGB						2
#define REMOTE_MUL_SWITCH_CCT					0
#define REMOTE_MUL_SWITCH_RGB					1
#define REMOTE_MUL_TIMER_EN						1
#define REMOTE_MUL_TIMER_DIS					2

typedef enum{
	enum_remoteMul_ConfigGroup,
	enum_remoteMul_Scene_Set,
	enum_remoteMul_Scene_Del
}remote_Cmd_t;

typedef enum{
	enum_rspRemoteMul_OnoffGroup,
	enum_rspRemoteMul_ActiveScene,
	enum_rspRemoteMul_DimCctAll,
	enum_rspRemoteMul_DimGroup,
	enum_rspRemoteMul_CctGroup,
	enum_rspRemoteMul_HslGroup,
	enum_rspRemoteMul_Timer
}remote_Rsp_t;

void RemoteMul_Cmd(remote_Cmd_t typeCmd, uint16_t adr, uint16_t idGroup, uint8_t idButton, uint16_t idScene);

void RemoteMul_Rsp_OnOffGroup(TS_GWIF_IncomingData *data);

void RemoteMul_Rsp_CallScene(TS_GWIF_IncomingData * data) ;

void RemoteMul_Rsp_TIMER(TS_GWIF_IncomingData * data);

void RemoteMul_Rsp_CCTDIMRGB(TS_GWIF_IncomingData * data);

void RemoteMul_Rsp_SwitchStatusLight(TS_GWIF_IncomingData * data);

void RemoteMul_Rsp_CallSceneDefault(TS_GWIF_IncomingData * data);

#endif /* REMOTEMULTIBUTTONS_REMOTEMULTIBUTTONS_H_ */

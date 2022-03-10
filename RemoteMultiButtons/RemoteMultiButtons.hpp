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

#define REMOTE_MUL_CONFIG_GROUP		0x0020
#define REMOTE_MUL_SCENE_SET		0x0020
#define REMOTE_MUL_SCENE_DEL		0x0020


/*Remote multi button rsp when press button*/
#define REMOTE_MUL_ONOFF_GROUP		0x0020  
#define REMOTE_MUL_SCENE_ACTIVE		0x0120
#define REMOTE_MUL_DIM_CCT_ALL		0x0220
#define REMOTE_MUL_DIM_GROUP		0x0320
#define REMOTE_MUL_CCT_GROUP		0x0420
#define REMOTE_MUL_HSL_GROUP		0x0520
#define REMOTE_MUL_TIMER			0x0620

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

#endif /* REMOTEMULTIBUTTONS_REMOTEMULTIBUTTONS_H_ */

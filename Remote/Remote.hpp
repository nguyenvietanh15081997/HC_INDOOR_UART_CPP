/*
 * Remote.h
 *
 *  Created on: Oct 29, 2021
 *      Author: anh
 */

#ifndef REMOTE_REMOTE_HPP_
#define REMOTE_REMOTE_HPP_

#include "../Include/Include.hpp"
#include "../logging/slog.h"

#define REMOTE_DC								(0x0002)
#define REMOTE_AC								(0x0003)
#define HEADER_SCENE_REMOTE_AC_SET 				(0x0103)
#define HEADER_SCENE_REMOTE_AC_DEL 				(0x0203)
#define HEADER_CONTROL_RGB_AC					(0x0503)
#define HEADER_SCENE_REMOTE_DC_SET 				(0x0102)
#define HEADER_SCENE_REMOTE_DC_DEL 				(0x0202)

typedef struct remotersp{
	uint8_t   	typeDev[2];
	uint8_t   	buttonID;
	uint8_t   	modeID;
	uint16_t  	sceneId;
	uint8_t   	futureID[2];
}remotersp_t;

typedef struct remoteRspAddScene{
	uint16_t 	typeRemote;
	uint8_t 	buttonId;
	uint8_t 	modeId;
	uint16_t	sceneId;
}remoteRspAddScene_t;

typedef struct remotrRspDelScene{
	uint16_t 	typeRemote;
	uint8_t 	buttonId;
	uint8_t 	modeId;
	uint16_t 	sceneId;
}remotrRspDelScene_t;

typedef struct remotePowerStatus{
	uint16_t 	header;
	uint8_t 	battery;
}remotePowerStatus_t;

void RspRemoteStatus(TS_GWIF_IncomingData *data);

void RspAddSceneRemote(TS_GWIF_IncomingData *data);

void RspRemoteDelScene(TS_GWIF_IncomingData *data);

void RspPowerRemoteStatus(TS_GWIF_IncomingData *data);

void RspControlRGB(TS_GWIF_IncomingData *data);

#endif /* REMOTE_REMOTE_H_ */

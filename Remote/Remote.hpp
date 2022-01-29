/*
 * Remote.h
 *
 *  Created on: Oct 29, 2021
 *      Author: anh
 */

#ifndef REMOTE_REMOTE_HPP_
#define REMOTE_REMOTE_HPP_

#include "../Include/Include.hpp"

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

#endif /* REMOTE_REMOTE_H_ */

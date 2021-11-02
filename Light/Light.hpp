/*
 * Light.h
 *
 *  Created on: Oct 29, 2021
 *      Author: anh
 */

#ifndef LIGHT_LIGHT_HPP_
#define LIGHT_LIGHT_HPP_

#include "../Include/Include.hpp"

void RspOnoff(TS_GWIF_IncomingData *data);

void RspCCT(TS_GWIF_IncomingData *data);

void RspDIM(TS_GWIF_IncomingData *data);

void RspHSL(TS_GWIF_IncomingData *data);

void RspAddDelGroup(TS_GWIF_IncomingData *data);

void RspAddDelSceneLight(TS_GWIF_IncomingData *data);

void RspCallScene(TS_GWIF_IncomingData *data) ;

void RspCallModeRgb(TS_GWIF_IncomingData *data);

void RspSaveGw(TS_GWIF_IncomingData *data);

void RspTypeDevice(TS_GWIF_IncomingData *data);

void RspResetNode(TS_GWIF_IncomingData *data);

#endif /* LIGHT_LIGHT_H_ */

/*
 * Include.cpp
 *
 *  Created on: Oct 30, 2021
 *      Author: anh
 */

#include "Include.hpp"

using namespace std;
queue<uartSendDev_t> 	bufferDataUart;
cmdcontrol_t 			vrts_CMD_STRUCTURE;
bool 					gvrb_AddSceneLight;
bool 					gvrb_AddGroupLight;
bool					gvrb_Provision;
uint16_t 				gSceneIdDel;
string 					gMacDev;
string 					gUUID;
string 					gDeviceKey;
string 					gNetKey;
string 					gAppKey;
string 					gFirmwareVer = "1.0";

int ptempIndoor;
uint16_t phumIndoor;
uint16_t ppm25;

uartSendDev_t AssignData(uint8_t *data,int length){
	uartSendDev_t dataSendUart;
	dataSendUart.length = length;
	memcpy((uint8_t *)(&dataSendUart.dataUart.HCI_CMD_GATEWAY[0]), data, length);
	dataSendUart.timeWait = 0;
//	for (int i = 0; i < length; i++) {
//		printf("%x ", data[i]);
//	}
//	printf("\n");
	return dataSendUart;
}


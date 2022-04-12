/*
 * Include.cpp
 *
 *  Created on: Oct 30, 2021
 *      Author: anh
 */

#include "Include.hpp"

using namespace std;
deque<uartSendDev_t> 	bufferDataUart;
deque<uartSendDev_t>    bufferUartUpdate;
cmdcontrol_t 			vrts_CMD_STRUCTURE;
bool 					gvrb_AddSceneLight;
bool 					gvrb_AddGroupLight;
bool					gvrb_Provision;
uint16_t 				gSceneIdDel;
pthread_mutex_t vrpth_SendUart = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t vrpth_DelScene = PTHREAD_MUTEX_INITIALIZER;

bool startProcessRoom = false;

int ptempIndoor;
uint16_t phumIndoor;
uint16_t ppm25;

uint16_t g_listAdrScene[MAX_DEV][2];


typeCmd_t vrte_TypeCmd;

uartSendDev_t AssignData(uint8_t *data,int length){
	uartSendDev_t dataSendUart;
	dataSendUart.length = length;
	memcpy((uint8_t *)(&dataSendUart.dataUart.HCI_CMD_GATEWAY[0]), data, length);
	dataSendUart.timeWait = 0;
#if 0
	for (int i = 0; i < length; i++) {
		printf("%x ", data[i]);
	}
	printf("\n");
#endif
	return dataSendUart;
}


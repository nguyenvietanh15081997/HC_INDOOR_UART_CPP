/*
 * Include.cpp
 *
 *  Created on: Oct 30, 2021
 *      Author: anh
 */

#include "Include.hpp"

using namespace std;
uartSendDev_t CmdPir[MAX_PIR] = {0,0,0};
deque<uartSendDev_t> 	bufferDataUart;
deque<uartSendDev_t>    bufferUartUpdate;
deque<string> 			bufferSendMqtt;
cmdcontrol_t 			vrts_CMD_STRUCTURE;
bool 					gvrb_AddSceneLight;
bool 					gvrb_AddGroupLight;
bool					gvrb_Provision;
uint16_t 				gSceneIdDel;
pthread_mutex_t vrpth_SendUart = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t vrpth_DelScene = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t vrpth_SendMqtt = PTHREAD_MUTEX_INITIALIZER;

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
	dataSendUart.timeWait = 10;
#if 0
	for (int i = 0; i < length; i++) {
		printf("%x ", data[i]);
	}
	printf("\n");
#endif
	return dataSendUart;
}

void Data2BufferSendMqtt(string s) {
	while(pthread_mutex_trylock(&vrpth_SendMqtt) != 0){};
	bufferSendMqtt.push_back(s);
	pthread_mutex_unlock(&vrpth_SendMqtt);
}

int Push2BufPirCmd(uartSendDev_t data){
	for(int i = 0; i < MAX_PIR; i++){
		if(CmdPir[i].length == 0) {
			CmdPir[i] = data;
			return 1;
		}
	}
	return 0;
}


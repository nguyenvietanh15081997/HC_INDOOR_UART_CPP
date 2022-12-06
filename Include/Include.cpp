/*
 * Include.cpp
 *
 *  Created on: Oct 30, 2021
 *      Author: anh
 */

#include "Include.hpp"

using namespace std;

vector<bufPir_t> 		vt_Pir;
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

checkPir_t vrtsCheckPir;

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
	return dataSendUart;
}

void Data2BufferSendMqtt(string s) {
	while(pthread_mutex_trylock(&vrpth_SendMqtt) != 0){};
	bufferSendMqtt.push_back(s);
	pthread_mutex_unlock(&vrpth_SendMqtt);
}

bool DelItemForBufPirCmd(bufPir_t data){
	vector<bufPir_t>::iterator iter_name;
	for (iter_name = vt_Pir.begin(); iter_name != vt_Pir.end(); iter_name++)
	{
		if (iter_name->adr == data.adr &&
				iter_name->idScene == data.idScene &&
				iter_name->type == data.type){
			vt_Pir.erase(iter_name);
			return true;
		}
	}
	return false;
}

bufPir_t FindBufPir(uint16_t adr)
{
	bufPir_t temp;
	vector<bufPir_t>::iterator iter_name;
	for (iter_name = vt_Pir.begin(); iter_name != vt_Pir.end(); iter_name++)
	{
		if (iter_name->adr == adr){
			temp.adr = iter_name->adr;
			temp.data = iter_name->data;
			temp.idScene = iter_name->idScene;
			temp.type = iter_name->type;
			return temp;
		}
	}
	return temp;
}

void Push2BufSendUart(uartSendDev_t vrts_DataUartSend)
{
	while (pthread_mutex_trylock(&vrpth_SendUart) != 0){};
	bufferDataUart.push_back(vrts_DataUartSend);
	pthread_mutex_unlock(&vrpth_SendUart);
}



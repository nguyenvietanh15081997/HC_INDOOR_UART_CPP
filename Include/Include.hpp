
#ifndef INCLUDE_HPP_
#define INCLUDE_HPP_

#include <iostream>
#include <sstream>
#include <queue>
#include <deque>
#include <string>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include "../ProcessUart/RingBuffer.h"
#include "../rapidjson/document.h"
#include "../rapidjson/prettywriter.h"

using namespace std;

#define PRINTUART			1

extern bool 				gvrb_AddSceneLight;
extern bool 				gvrb_AddGroupLight;
extern bool					gvrb_Provision;
extern uint16_t 			gSceneIdDel;

#define SIZE_BUF_UARTSEND	4096
extern ringbuffer_t 		bufferDataUart;
extern pthread_mutex_t 		keyBufferUartSend;


extern ringbuffer_t         bufferSendMqtt;
extern pthread_mutex_t 		keyBufferSendMqtt;

extern int ptempIndoor;
extern uint16_t phumIndoor;
extern uint16_t ppm25;

/*
 * buffer send mqtt
 */
#define MAX_CHAR_MQTT		2048
#define MAX_BUFFEMQTTSEND	2048
typedef struct dataSendMqtt{
	char dataSendMqtt[MAX_CHAR_MQTT];
}dataSendMqtt_t;

/*
 * frame data to control device
 */
typedef struct{
	uint8_t HCI_CMD_GATEWAY[2];// CMD
	uint8_t opCode00[4];   // 00 00 00 00
	uint8_t retry_cnt;     // num of send
	uint8_t rsp_max;       // num of rsp
	uint8_t adr_dst[2];    // adr
	uint8_t opCode[2];
	uint8_t para[68];
} cmdcontrol_t;
extern cmdcontrol_t 	vrts_CMD_STRUCTURE;

/*
 * frame data use opcode vendor
 */
#if 0
typedef struct{
	uint8_t HCI_CMD_GATEWAY[2];// CMD
	uint8_t opCode00[4];   // 00 00 00 00
	uint8_t retry_cnt;     // num of send
	uint8_t rsp_max;       // num of rsp
	uint8_t adr_dst[2];    // adr
	uint8_t opCode[3];     // opcode vendor
	uint8_t status_cmd[2]; // status cmd
	uint8_t para[67];
} cmdcontrol_vendor;
#endif

/*
 * Frame data uart rsp
 */
#define MESSAGE_MAXLENGTH		(61)
typedef struct IncomingData{
	uint8_t 	Length[2];
	uint8_t 	Opcode;
	uint8_t		Message[MESSAGE_MAXLENGTH];
} TS_GWIF_IncomingData;

/*
 * Item buffer data uart cmd
 *
 * length: num byte send
 * datauart: data send
 * timeWait: time wait to send dataUart
 */
typedef struct uartSendDev{
	uint16_t 		length;
	cmdcontrol_t 	dataUart;
	uint64_t 		timeWait;
}uartSendDev_t;

uartSendDev_t AssignData(uint8_t *data,int length);

#endif /* INCLUDE_INCLUDE_HPP_ */

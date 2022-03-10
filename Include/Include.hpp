
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
#include "../rapidjson/document.h"
#include "../rapidjson/prettywriter.h"

using namespace std;

#define PRINTUART			1

#define PROPERTY_ONOFF		0
#define PROPERTY_DIM		1
#define PROPERTY_CCT		2
#define PROPERTY_H			3
#define PROPERTY_S			4
#define PROPERTY_L			5
#define PROPERTY_SONG		6
#define PROPERTY_BLINK_MODE	7
#define PROPERTY_BATTERY	8
#define PROPERTY_LUX		9
#define PROPERTY_PIR		10
#define PROPERTY_BUTTON1	11
#define PROPERTY_BUTTON2	12
#define PROPERTY_BUTTON3	13
#define PROPERTY_BUTTON4	14

extern bool 				gvrb_AddSceneLight;
extern bool 				gvrb_AddGroupLight;
extern bool					gvrb_Provision;
extern uint16_t 			gSceneIdDel;
extern bool 				gvrb_UpdateONOFF;
extern bool 				gvrb_UpdateHSL;
extern bool 				startProcessRoom ;

extern int ptempIndoor;
extern uint16_t phumIndoor;
extern uint16_t ppm25;
extern pthread_mutex_t vrpth_SendUart;

#define MAX_DEV    500
extern uint16_t g_listAdrScene[MAX_DEV][2];

typedef enum {
	typeCmd_Control,
	typeCmd_Update
}typeCmd_t;
extern typeCmd_t vrte_TypeCmd;

/*frame data to control device*/
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

/*frame data use opcode vendor*/
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

#define MESSAGE_MAXLENGTH		(61)
typedef struct IncomingData{
	uint8_t 	Length[2];
	uint8_t 	Opcode;
	uint8_t		Message[MESSAGE_MAXLENGTH];
} TS_GWIF_IncomingData;

typedef struct uartSendDev{
	uint16_t 		length;
	cmdcontrol_t 	dataUart;
	uint64_t 		timeWait;
}uartSendDev_t;

typedef struct uartSendDev_Vendor{
	uint8_t 			length;
	cmdcontrol_vendor 	dataUart;
	long 				timeWait;
}uartSendDev_Vendor_t;

#define MAX_MSG_MQTT 		512
typedef struct bufferDataMqtt{
	char dataMqtt[MAX_MSG_MQTT];
}bufferDataMqtt_t;

extern deque<uartSendDev_t> bufferDataUart;
extern deque<uartSendDev_t> bufferUartUpdate;

uartSendDev_t AssignData(uint8_t *data,int length);

#endif /* INCLUDE_INCLUDE_HPP_ */

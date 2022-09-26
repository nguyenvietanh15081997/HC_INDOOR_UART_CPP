/*
 * SwitchOnOff.c
 *
 *  Created on: Jan 24, 2022
 *      Author: anh
 */

#include "SwitchOnOff.hpp"
#include "../Mqtt/Mqtt.hpp"
#include "../ProcessUart/OpCode.h"

using namespace std;
using namespace rapidjson;

#define SWITCH_LENGTH_CONTROL			23
#define SWITCH_LENGTH_SCENE_SET			23
#define SWITCH_LENGTH_SCENE_DEL			19
#define SWITCH_LENGTH_STATUS			17
#define SWITCH_LENGTH_CONTROL_HSL		23
#define SWITCH_LENGTH_CONTROL_COMBINE	23
#define SWITCH_LENGTH_TIMER				23

#define TYPE_BNL					22005

static uint16_t switch_cmd = HCI_CMD_GATEWAY_CMD;
static uint8_t switch_parRetry_cnt = 0x00;
static uint8_t switch_parRsp_Max = 0x00;

typedef enum {
	switch1,
	binhnonglanh,
	nullDev
}typeDev_e;
typeDev_e typeDev = nullDev;

static void Switch_Control(uint16_t typeSwitch, uint16_t adr, uint8_t relayId, uint8_t relayValue){
	uint16_t opcode;
	if (typeSwitch == SWITCH_1_TYPE || typeSwitch == BLN_TYPE){
		opcode = SWITCH_1_CONTROL;
	} else if(typeSwitch == SWITCH_2_TYPE){
		opcode = SWITCH_2_CONTROL;
	} else if(typeSwitch == SWITCH_3_TYPE ){
		opcode = SWITCH_3_CONTROL;
	} else if (typeSwitch == SWITCH_4_TYPE
			|| typeSwitch == SWITCH_RGB_4_TYPE
			|| typeSwitch == SWITCH_RGB_3_TYPE
			|| typeSwitch == SWITCH_RGB_2_TYPE
			|| typeSwitch == SWITCH_RGB_1_TYPE
			|| typeSwitch == BLN_RGB_TYPE
			|| typeSwitch == CONG_TAC_CO_1_TYPE
			|| typeSwitch == CONG_TAC_CO_2_TYPE
			|| typeSwitch == CONG_TAC_CO_3_TYPE
			|| typeSwitch == CONG_TAC_CO_4_TYPE ) {
		opcode = SWITCH_4_CONTROL;
	}
	vrts_CMD_STRUCTURE.adr_dst[0] = adr & 0xFF;
	vrts_CMD_STRUCTURE.adr_dst[1] = (adr>>8) & 0xFF;
	vrts_CMD_STRUCTURE.opCode[0] = RD_OPCODE_SCENE_SEND & 0xFF;
	vrts_CMD_STRUCTURE.opCode[1] = VENDOR_ID & 0xFF;
	vrts_CMD_STRUCTURE.para[0] = (VENDOR_ID>>8) & 0xFF;
	vrts_CMD_STRUCTURE.para[1] = STATUS_CMD_SCENE & 0xFF;
	vrts_CMD_STRUCTURE.para[2] = (STATUS_CMD_SCENE>>8) & 0xFF;
	vrts_CMD_STRUCTURE.para[3] = opcode & 0xFF;
	vrts_CMD_STRUCTURE.para[4] = (opcode >>8) & 0xFF;
	vrts_CMD_STRUCTURE.para[5] = relayId & 0xFF;
	vrts_CMD_STRUCTURE.para[6] = relayValue & 0xFF;
}

static void Switch_Scene_Set(uint16_t typeSwitch, uint16_t adr, uint8_t relay1,
		uint8_t relay2, uint8_t relay3, uint8_t relay4, uint16_t sceneId) {
	uint16_t opcode;
	if (typeSwitch == SWITCH_1_TYPE
			|| typeSwitch == BLN_TYPE ){
		opcode = SWITCH_1_SCENE_SET;
	} else if(typeSwitch == SWITCH_2_TYPE){
		opcode = SWITCH_2_SCENE_SET;
	} else if(typeSwitch == SWITCH_3_TYPE){
		opcode = SWITCH_3_SCENE_SET;
	} else if(typeSwitch == SWITCH_4_TYPE){
		opcode = SWITCH_4_SCENE_SET;
	}
	vrts_CMD_STRUCTURE.adr_dst[0] = adr & 0xFF;
	vrts_CMD_STRUCTURE.adr_dst[1] = (adr >> 8) & 0xFF;
	vrts_CMD_STRUCTURE.opCode[0] = RD_OPCODE_SCENE_SEND & 0xFF;
	vrts_CMD_STRUCTURE.opCode[1] = VENDOR_ID & 0xFF;
	vrts_CMD_STRUCTURE.para[0] = (VENDOR_ID >> 8) & 0xFF;
	vrts_CMD_STRUCTURE.para[1] = STATUS_CMD_SCENE & 0xFF;
	vrts_CMD_STRUCTURE.para[2] = (STATUS_CMD_SCENE >> 8) & 0xFF;
	vrts_CMD_STRUCTURE.para[3] = opcode & 0xFF;
	vrts_CMD_STRUCTURE.para[4] = (opcode >> 8) & 0xFF;
	vrts_CMD_STRUCTURE.para[5] = sceneId & 0xFF;
	vrts_CMD_STRUCTURE.para[6] = (sceneId >> 8) & 0xFF;
	vrts_CMD_STRUCTURE.para[7] = relay1 & 0xFF;
	vrts_CMD_STRUCTURE.para[8] = relay2 & 0xFF;
	vrts_CMD_STRUCTURE.para[9] = relay3 & 0xFF;
	vrts_CMD_STRUCTURE.para[10] = relay4 & 0xFF;
}

static void Switch_Scene_Del(uint16_t typeSwitch, uint16_t adr, uint16_t sceneId){
	uint16_t opcode;
	if (typeSwitch == SWITCH_1_TYPE
			|| typeSwitch == BLN_TYPE ){
		opcode = SWITCH_1_SCENE_DEL;
	} else if(typeSwitch == SWITCH_2_TYPE ){
		opcode = SWITCH_2_SCENE_DEL;
	} else if(typeSwitch == SWITCH_3_TYPE ){
		opcode = SWITCH_3_SCENE_DEL;
	} else if(typeSwitch == SWITCH_4_TYPE ){
		opcode = SWITCH_4_SCENE_DEL;
	}
	vrts_CMD_STRUCTURE.adr_dst[0] = adr & 0xFF;
	vrts_CMD_STRUCTURE.adr_dst[1] = (adr >> 8) & 0xFF;
	vrts_CMD_STRUCTURE.opCode[0] = RD_OPCODE_SCENE_SEND & 0xFF;
	vrts_CMD_STRUCTURE.opCode[1] = VENDOR_ID & 0xFF;
	vrts_CMD_STRUCTURE.para[0] = (VENDOR_ID >> 8) & 0xFF;
	vrts_CMD_STRUCTURE.para[1] = STATUS_CMD_SCENE & 0xFF;
	vrts_CMD_STRUCTURE.para[2] = (STATUS_CMD_SCENE >> 8) & 0xFF;
	vrts_CMD_STRUCTURE.para[3] = opcode & 0xFF;
	vrts_CMD_STRUCTURE.para[4] = (opcode >> 8) & 0xFF;
	vrts_CMD_STRUCTURE.para[5] = (sceneId) & 0xFF;
	vrts_CMD_STRUCTURE.para[6] = (sceneId >> 8) & 0xFF;
}

static void Switch_RequestStatus(uint16_t typeSwitch, uint16_t adr){
	uint16_t opcode;
	if (typeSwitch == SWITCH_1_TYPE
			|| typeSwitch == BLN_TYPE){
		opcode = SWITCH_1_STATUS;
	} else if(typeSwitch == SWITCH_2_TYPE ){
		opcode = SWITCH_2_STATUS;
	} else if(typeSwitch == SWITCH_3_TYPE ){
		opcode = SWITCH_3_STATUS;
	} else if (typeSwitch == SWITCH_4_TYPE){
		opcode = SWITCH_4_STATUS;
	} else if( typeSwitch == SWITCH_RGB_4_TYPE
			|| typeSwitch == SWITCH_RGB_3_TYPE
			|| typeSwitch == SWITCH_RGB_2_TYPE
			|| typeSwitch == SWITCH_RGB_1_TYPE
			|| typeSwitch == BLN_RGB_TYPE
			|| typeSwitch == CONG_TAC_CO_1_TYPE
			|| typeSwitch == CONG_TAC_CO_2_TYPE
			|| typeSwitch == CONG_TAC_CO_3_TYPE
			|| typeSwitch == CONG_TAC_CO_4_TYPE ){
		opcode = REQUEST_STATUS_DEV_RGB;
	}
	vrts_CMD_STRUCTURE.adr_dst[0] = adr & 0xFF;
	vrts_CMD_STRUCTURE.adr_dst[1] = (adr >> 8) & 0xFF;
	vrts_CMD_STRUCTURE.opCode[0] = RD_OPCODE_SCENE_SEND & 0xFF;
	vrts_CMD_STRUCTURE.opCode[1] = VENDOR_ID & 0xFF;
	vrts_CMD_STRUCTURE.para[0] = (VENDOR_ID >> 8) & 0xFF;
	vrts_CMD_STRUCTURE.para[1] = STATUS_CMD_SCENE & 0xFF;
	vrts_CMD_STRUCTURE.para[2] = (STATUS_CMD_SCENE >> 8) & 0xFF;
	vrts_CMD_STRUCTURE.para[3] = opcode & 0xFF;
	vrts_CMD_STRUCTURE.para[4] = (opcode >> 8) & 0xFF;
}


static void Switch_ControlHSL(uint16_t adr,uint16_t type, uint8_t button, uint8_t b, uint8_t g, uint8_t r, uint8_t dimOn, uint8_t dimOff) {
	uint16_t opcode = SWITCH_4_CONTROL_HSL;
	vrts_CMD_STRUCTURE.adr_dst[0] = adr & 0xFF;
	vrts_CMD_STRUCTURE.adr_dst[1] = (adr >> 8) & 0xFF;
	vrts_CMD_STRUCTURE.opCode[0] = RD_OPCODE_SCENE_SEND & 0xFF;
	vrts_CMD_STRUCTURE.opCode[1] = VENDOR_ID & 0xFF;
	vrts_CMD_STRUCTURE.para[0] = (VENDOR_ID >> 8) & 0xFF;
	vrts_CMD_STRUCTURE.para[1] = STATUS_CMD_SCENE & 0xFF;
	vrts_CMD_STRUCTURE.para[2] = (STATUS_CMD_SCENE >> 8) & 0xFF;
	vrts_CMD_STRUCTURE.para[3] = opcode & 0xFF;
	vrts_CMD_STRUCTURE.para[4] = (opcode >> 8) & 0xFF;
	vrts_CMD_STRUCTURE.para[5] = button;
	vrts_CMD_STRUCTURE.para[6] = b;
	vrts_CMD_STRUCTURE.para[7] = g;
	vrts_CMD_STRUCTURE.para[8] = r;
	vrts_CMD_STRUCTURE.para[9] = dimOn;
	vrts_CMD_STRUCTURE.para[10] = dimOff;
}

static void Switch_ControlCombine(uint16_t adr, uint16_t type, uint16_t id){
	uint16_t opcode = SWITCH_4_CONTROL_COMBINE;
	vrts_CMD_STRUCTURE.adr_dst[0] = adr & 0xFF;
	vrts_CMD_STRUCTURE.adr_dst[1] = (adr >> 8) & 0xFF;
	vrts_CMD_STRUCTURE.opCode[0] = RD_OPCODE_SCENE_SEND & 0xFF;
	vrts_CMD_STRUCTURE.opCode[1] = VENDOR_ID & 0xFF;
	vrts_CMD_STRUCTURE.para[0] = (VENDOR_ID >> 8) & 0xFF;
	vrts_CMD_STRUCTURE.para[1] = STATUS_CMD_SCENE & 0xFF;
	vrts_CMD_STRUCTURE.para[2] = (STATUS_CMD_SCENE >> 8) & 0xFF;
	vrts_CMD_STRUCTURE.para[3] = opcode & 0xFF;
	vrts_CMD_STRUCTURE.para[4] = (opcode >> 8) & 0xFF;
	vrts_CMD_STRUCTURE.para[5] = id & 0xFF;
	vrts_CMD_STRUCTURE.para[6] = (id >> 8) & 0xFF;
	vrts_CMD_STRUCTURE.para[7] = 0;
	vrts_CMD_STRUCTURE.para[8] = 0;
	vrts_CMD_STRUCTURE.para[9] = 0;
	vrts_CMD_STRUCTURE.para[10] = 0;
}

static void Switch_Timer(uint16_t adr, uint16_t type, uint8_t status, uint32_t time){
	uint16_t opcode = SWITCH_4_TIMER;
	vrts_CMD_STRUCTURE.adr_dst[0] = adr & 0xFF;
	vrts_CMD_STRUCTURE.adr_dst[1] = (adr >> 8) & 0xFF;
	vrts_CMD_STRUCTURE.opCode[0] = RD_OPCODE_SCENE_SEND & 0xFF;
	vrts_CMD_STRUCTURE.opCode[1] = VENDOR_ID & 0xFF;
	vrts_CMD_STRUCTURE.para[0] = (VENDOR_ID >> 8) & 0xFF;
	vrts_CMD_STRUCTURE.para[1] = STATUS_CMD_SCENE & 0xFF;
	vrts_CMD_STRUCTURE.para[2] = (STATUS_CMD_SCENE >> 8) & 0xFF;
	vrts_CMD_STRUCTURE.para[3] = opcode & 0xFF;
	vrts_CMD_STRUCTURE.para[4] = (opcode >> 8) & 0xFF;
	vrts_CMD_STRUCTURE.para[5] = status & 0xFF;
	vrts_CMD_STRUCTURE.para[6] = (time >> 24) & 0xFF;
	vrts_CMD_STRUCTURE.para[7] = (time >> 16) & 0xFF;
	vrts_CMD_STRUCTURE.para[8] = (time >> 8) & 0xFF;
	vrts_CMD_STRUCTURE.para[9] = time & 0xFF;
	vrts_CMD_STRUCTURE.para[10] = 0;
}

static void InitFramUart(){
	vrts_CMD_STRUCTURE.HCI_CMD_GATEWAY[0]	= switch_cmd & 0xFF;
	vrts_CMD_STRUCTURE.HCI_CMD_GATEWAY[1]	= (switch_cmd>>8) & 0xFF;
	vrts_CMD_STRUCTURE.opCode00[0] 			= 0x00;
	vrts_CMD_STRUCTURE.opCode00[1]			= 0x00;
	vrts_CMD_STRUCTURE.opCode00[2] 			= 0x00;
	vrts_CMD_STRUCTURE.opCode00[3]			= 0x00;
	vrts_CMD_STRUCTURE.retry_cnt   			= switch_parRetry_cnt;
	vrts_CMD_STRUCTURE.rsp_max     			= switch_parRsp_Max;
}

#define SWITCH_TIME_WAIT		500
#define SWITCH_TIME_WAIT_UPD	3000
void Switch_Send_Uart(switch_enum_cmd typeCmd, uint16_t typeSw, uint16_t adr,
		uint8_t relayId, uint8_t relayValue, uint8_t relay1, uint8_t relay2,
		uint8_t relay3, uint8_t relay4, uint16_t sceneId, uint8_t r, uint8_t g, uint8_t b, uint32_t time) {
	uint16_t cmdLength = 0;
	uint64_t sw_timewait;
	InitFramUart();
	if(typeCmd == switch_enum_control){
		Switch_Control(typeSw, adr, relayId, relayValue);
		cmdLength = SWITCH_LENGTH_CONTROL;
		sw_timewait = SWITCH_TIME_WAIT;
	} else if (typeCmd == switch_enum_addscene) {
		Switch_Scene_Set(typeSw, adr, relay1, relay2, relay3, relay4, sceneId);
		cmdLength = SWITCH_LENGTH_SCENE_SET;
		sw_timewait = SWITCH_TIME_WAIT;
	} else if (typeCmd == switch_enum_delscene) {
		Switch_Scene_Del(typeSw, adr, sceneId);
		cmdLength = SWITCH_LENGTH_SCENE_DEL;
		sw_timewait = SWITCH_TIME_WAIT;
	} else if (typeCmd == switch_enum_status){
		Switch_RequestStatus(typeSw,adr);
		cmdLength = SWITCH_LENGTH_STATUS;
		sw_timewait = SWITCH_TIME_WAIT_UPD;
	} else if (typeCmd == switch_enum_control_hsl) {
		Switch_ControlHSL(adr,typeSw, relayId, b, g, r, relay1, relay2);
		sw_timewait = SWITCH_TIME_WAIT_UPD;
		cmdLength = SWITCH_LENGTH_CONTROL_HSL;
	} else if (typeCmd == switch_enum_control_combine) {
		Switch_ControlCombine(adr, typeSw, sceneId);
		sw_timewait = SWITCH_TIME_WAIT_UPD;
		cmdLength = SWITCH_LENGTH_CONTROL_COMBINE;
	} else if (typeCmd ==  switch_enum_timer) {
		Switch_Timer(adr, typeSw, relayId, time);
		sw_timewait = SWITCH_TIME_WAIT_UPD;
		cmdLength = SWITCH_LENGTH_TIMER;
	}

	uartSendDev_t vrts_DataUartSend;
	vrts_DataUartSend.length = cmdLength;
	vrts_DataUartSend.dataUart = vrts_CMD_STRUCTURE;
	vrts_DataUartSend.timeWait = sw_timewait;

	while(pthread_mutex_trylock(&vrpth_SendUart) != 0){};
	if (typeCmd == switch_enum_status){
		bufferUartUpdate.push_back(vrts_DataUartSend);
	} else {
		bufferDataUart.push_back(vrts_DataUartSend);
	}
	pthread_mutex_unlock(&vrpth_SendUart);
}

void Rsp_Switch_Control(TS_GWIF_IncomingData * data){
	uint16_t adr = data->Message[1] | (data->Message[2] << 8);
	uint16_t opcode = data->Message[8] | (data->Message[9] << 8);
	uint8_t relayId = data->Message[10];
	uint8_t relayValue = data->Message[11];
	uint16_t type;
	if (opcode == SWITCH_1_CONTROL){
		type = SWITCH_1_TYPE;
	} else if (opcode == SWITCH_2_CONTROL){
		type = SWITCH_2_TYPE;
	} else if (opcode == SWITCH_3_CONTROL){
		type = SWITCH_3_TYPE;
	} else if (opcode == SWITCH_4_CONTROL){
		type = SWITCH_4_TYPE;
	}

	StringBuffer dataMqtt;
	Writer<StringBuffer> json(dataMqtt);
	json.StartObject();
		json.Key("CMD");json.String("CONTROL_SWITCH");
		json.Key("TYPE");json.Int(type);
		json.Key("DATA");
		json.StartObject();
			json.Key("DEVICE_UNICAST_ID");json.Int(adr);
			json.Key("RELAY_ID");json.Int(relayId);
			json.Key("RELAY_STATUS");json.Int(relayValue);
		json.EndObject();
	json.EndObject();

//	cout << dataMqtt.GetString() << endl;
	string s = dataMqtt.GetString();
	slog_info("<mqtt>send: %s", s.c_str());
	Data2BufferSendMqtt(s);
}

void Rsp_Switch_Scene_Set(TS_GWIF_IncomingData * data){
	uint16_t adr = data->Message[1] | (data->Message[2] << 8);
	uint16_t opcode = data->Message[8] | (data->Message[9] << 8);
	uint16_t sceneId = data->Message[10] | (data->Message[11] << 8);
	uint8_t relay1 = data->Message[12];
	uint8_t relay2 = data->Message[13];
	uint8_t relay3 = data->Message[14];
	uint8_t relay4 = data->Message[15];
	uint16_t type;
	if (opcode == SWITCH_1_SCENE_SET){
		type = SWITCH_1_TYPE;
	} else if (opcode == SWITCH_2_SCENE_SET){
		type = SWITCH_2_TYPE;
	} else if (opcode == SWITCH_3_SCENE_SET){
		type = SWITCH_3_TYPE;
	} else if (opcode == SWITCH_4_SCENE_SET){
		type = SWITCH_4_TYPE;
	}

	StringBuffer dataMqtt;
	Writer<StringBuffer> json(dataMqtt);
	json.StartObject();
		json.Key("CMD");json.String("ADDSCENE_SWITCH");
		json.Key("TYPE");json.Int(type);
		json.Key("DATA");
		json.StartObject();
			json.Key("DEVICE_UNICAST_ID");json.Int(adr);
			json.Key("SWITCH_STATUS");
			json.StartObject();
				json.Key("RELAY1");json.Int(relay1);
				json.Key("RELAY2");json.Int(relay2);
				json.Key("RELAY3");json.Int(relay3);
				json.Key("RELAY4");json.Int(relay4);
			json.EndObject();
			json.Key("SCENEID");json.Int(sceneId);
		json.EndObject();
	json.EndObject();

//	cout << dataMqtt.GetString() << endl;
	string s = dataMqtt.GetString();
	slog_info("<mqtt>send: %s", s.c_str());
	Data2BufferSendMqtt(s);
}

void Rsp_Switch_Scene_Del(TS_GWIF_IncomingData * data){
	uint16_t adr = data->Message[1] | (data->Message[2] << 8);
	uint16_t opcode = data->Message[8] | (data->Message[9] << 8);
	uint16_t sceneId = data->Message[10] | (data->Message[11] << 8);

	uint16_t type;
	if (opcode == SWITCH_1_SCENE_DEL){
		type = SWITCH_1_TYPE;
	} else if (opcode == SWITCH_1_SCENE_DEL){
		type = SWITCH_2_TYPE;
	} else if (opcode == SWITCH_3_SCENE_DEL){
		type = SWITCH_3_TYPE;
	} else if (opcode == SWITCH_4_SCENE_DEL){
		type = SWITCH_4_TYPE;
	}

	StringBuffer dataMqtt;
	Writer<StringBuffer> json(dataMqtt);
	json.StartObject();
		json.Key("CMD");json.String("DELSCENE_SWITCH4");
		json.Key("TYPE");json.Int(type);
		json.Key("DATA");
		json.StartObject();
			json.Key("DEVICE_UNICAST_ID");json.Int(adr);
			json.Key("SCENEID");json.Int(sceneId);
		json.EndObject();
	json.EndObject();

//	cout << dataMqtt.GetString() << endl;
	string s = dataMqtt.GetString();
	slog_info("<mqtt>send: %s", s.c_str());
	Data2BufferSendMqtt(s);
}

void Rsp_Switch_Status(TS_GWIF_IncomingData * data){
	uint16_t adr = data->Message[1] | (data->Message[2] << 8);
	uint16_t opcode = data->Message[6] | (data->Message[7] << 8);
	uint8_t relayId = data->Message[8];
	uint8_t relayValue = data->Message[9];

	uint16_t type;
	if (opcode == SWITCH_1_CONTROL){
		type = SWITCH_1_TYPE;
	} else if (opcode == SWITCH_2_CONTROL){
		type = SWITCH_2_TYPE;
	} else if (opcode == SWITCH_3_CONTROL){
		type = SWITCH_3_TYPE;
	} else if (opcode == SWITCH_4_CONTROL){
		type = SWITCH_4_TYPE;
	}

	StringBuffer dataMqtt;
	Writer<StringBuffer> json(dataMqtt);
	json.StartObject();
		json.Key("CMD");json.String("SWITCH");
		json.Key("TYPE");json.Int(type);
		json.Key("DATA");
		json.StartObject();
			json.Key("DEVICE_UNICAST_ID");json.Int(adr);
			json.Key("RELAY_ID");json.Int(relayId);
			json.Key("RELAY_STATUS");json.Int(relayValue);
		json.EndObject();
	json.EndObject();

//	cout << dataMqtt.GetString() << endl;
	string s = dataMqtt.GetString();
	slog_info("<mqtt>send: %s", s.c_str());
	Data2BufferSendMqtt(s);
}

void Rsp_Switch_RequestStatus(TS_GWIF_IncomingData * data){
	uint16_t adr = data->Message[1] | (data->Message[2] << 8);
	uint16_t opcode = data->Message[8] | (data->Message[9] << 8);
	uint8_t listId[4] = {11,12,13,14};
	uint8_t listValue[4] = {data->Message[10], data->Message[11], data->Message[12], data->Message[13]};
	int index = 0;
	if (opcode == SWITCH_1_STATUS){
		index = 1;
	} else if (opcode == SWITCH_2_STATUS){
		index = 2;
	} else if (opcode == SWITCH_3_STATUS){
		index = 3;
	} else if (opcode == SWITCH_4_STATUS){
		index = 4;
	}
	StringBuffer dataMqtt;
	Writer<StringBuffer> json(dataMqtt);
	json.StartObject();
		json.Key("CMD");json.String("UPDATE");
		json.Key("DATA");
		json.StartObject();
			json.Key("DEVICE_UNICAST_ID");json.Int(adr);
			json.Key("PROPERTIES");
			json.StartArray();
			for(int i = 0; i < index ; i++) {
				json.StartObject();
					json.Key("ID"); json.Int(listId[i]);
					json.Key("VALUE"); json.Int(listValue[i]);
				json.EndObject();
			}
			json.EndArray();
		json.EndObject();
	json.EndObject();

//	cout << dataMqtt.GetString() << endl;
	string s = dataMqtt.GetString();
//	slog_info("<mqtt>send: %s", s.c_str());
	Data2BufferSendMqtt(s);
}

void Rsp_RequestStatusRgb(TS_GWIF_IncomingData * data){
	uint16_t adr = data->Message[1] | (data->Message[2] << 8);
	uint8_t element = data->Message[10];
	for(int i = 0; i < element; i++) {
		StringBuffer dataMqtt;
		Writer<StringBuffer> json(dataMqtt);
		json.StartObject();
			json.Key("CMD");json.String("UPDATE");
			json.Key("DATA");
			json.StartObject();
				json.Key("DEVICE_UNICAST_ID");json.Int(adr+i);
				json.Key("PROPERTIES");
				json.StartArray();
					json.StartObject();
						json.Key("ID"); json.Int(PROPERTY_ONOFF);
						json.Key("VALUE"); json.Int(data->Message[11+i]);
					json.EndObject();
				json.EndArray();
			json.EndObject();
		json.EndObject();

	//	cout << dataMqtt.GetString() << endl;
		string s = dataMqtt.GetString();
	//	slog_info("<mqtt>send: %s", s.c_str());
		Data2BufferSendMqtt(s);
	}
}

void Rsp_Switch_ControlRGB (TS_GWIF_IncomingData *data){
	cout << "into" << endl;
	uint16_t adr = data->Message[1] | (data->Message[2] << 8);
	uint8_t b = data->Message[11] ;
	uint8_t g = data->Message[12] ;
	uint8_t r = data->Message[13] ;
	uint8_t dimon = data->Message[14];
	uint8_t dimoff = data->Message[15];
	uint8_t btn = data->Message[10];
	uint8_t listProBtn[6] = {11,12,13,14,15,16};

	StringBuffer dataMqtt;
	Writer<StringBuffer> json(dataMqtt);
	json.StartObject();
		json.Key("CMD");json.String("DEVICE_CONTROL");
		json.Key("DATA");
		json.StartObject();
			json.Key("DEVICE_UNICAST_ID");json.Int(adr);
			json.Key("PROPERTIES");
			json.StartArray();
			json.StartObject();
				json.Key("ID"); json.Int(PROPERTY_R);
				json.Key("VALUE"); json.Int(r);
			json.EndObject();
			json.StartObject();
				json.Key("ID"); json.Int(PROPERTY_G);
				json.Key("VALUE"); json.Int(g);
			json.EndObject();
			json.StartObject();
				json.Key("ID"); json.Int(PROPERTY_B);
				json.Key("VALUE"); json.Int(b);
			json.EndObject();
			if (!btn){
				json.StartObject();
					json.Key("ID"); json.Int(listProBtn[btn-1]);
					json.Key("VALUE"); json.Int(1);
				json.EndObject();
			}
			json.StartObject();
				json.Key("ID"); json.Int(PROPERTY_DIMON);
				json.Key("VALUE"); json.Int(dimon);
			json.EndObject();
			json.StartObject();
				json.Key("ID"); json.Int(PROPERTY_DIMOFF);
				json.Key("VALUE"); json.Int(dimoff);
			json.EndObject();
			json.EndArray();
		json.EndObject();
	json.EndObject();

//	cout << dataMqtt.GetString() << endl;
	string s1 = dataMqtt.GetString();
	slog_info("<mqtt>send: %s", s1.c_str());
	Data2BufferSendMqtt(s1);
}

void Rsp_Switch_Control_Combine (TS_GWIF_IncomingData * data) {
	uint16_t adr = data->Message[1] | (data->Message[2] << 8);
	uint16_t id = data->Message[10] | (data->Message[11] << 8);

	StringBuffer dataMqtt;
	Writer<StringBuffer> json(dataMqtt);
	json.StartObject();
		json.Key("CMD");json.String("SWITCH_CONTROL_COMBINE");
		json.Key("DATA");
		json.StartObject();
			json.Key("DEVICE_UNICAST_ID");json.Int(adr);
			json.Key("ID"); json.Int(id);
		json.EndObject();
	json.EndObject();

//	cout << dataMqtt.GetString() << endl;
	string s1 = dataMqtt.GetString();
//	slog_info("<mqtt>send: %s", s.c_str());
	Data2BufferSendMqtt(s1);
}

void Rsp_Switch_Timer (TS_GWIF_IncomingData * data) {
	uint16_t adr = data->Message[1] | (data->Message[2] << 8);
	uint8_t status = data->Message[10];
	uint32_t time = (data->Message[11] << 24) | (data->Message[12] << 16) | (data->Message[13] << 8) | data->Message[14];

	StringBuffer dataMqtt;
	Writer<StringBuffer> json(dataMqtt);
	json.StartObject();
		json.Key("CMD");json.String("SWITCH_COUNTDOWN");
		json.Key("DATA");
		json.StartObject();
			json.Key("DEVICE_UNICAST_ID");json.Int(adr);
			json.Key("STATUS"); json.Int(status);
			json.Key("TIME"); json.Int64(time);
		json.EndObject();
	json.EndObject();

//	cout << dataMqtt.GetString() << endl;
	string s1 = dataMqtt.GetString();
//	slog_info("<mqtt>send: %s", s.c_str());
	Data2BufferSendMqtt(s1);
}

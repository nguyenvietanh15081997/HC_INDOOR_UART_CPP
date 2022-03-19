/*
 * Curtain.cpp
 *
 *  Created on: Feb 21, 2022
 *      Author: anh
 */
#include "Curtain.hpp"

using namespace std;
using namespace rapidjson;

#define CURTAIN_LENGTH_CONTROL			19
#define CURTAIN_LENGTH_STATUS_REQUEST	17
#define CURTAIN_LENGTH_SCENE_SET		19
#define CURTAIN_LENGTH_SCENE_DEL		19
#define CURTAIN_LENGTH_CALIB			19
#define CURTAIN_LENGTH_CONFIG_MOTOR		19

static uint16_t curtain_cmd = HCI_CMD_GATEWAY_CMD;
static uint8_t curtain_parRetry_cnt = 0x00;
static uint8_t curtain_parRsp_Max = 0x00;

static void CURTAIN_Control(uint8_t typeControl, uint8_t percentOpen) {
	vrts_CMD_STRUCTURE.opCode[0] = RD_OPCODE_SCENE_SEND & 0xFF;
	vrts_CMD_STRUCTURE.opCode[1] = VENDOR_ID & 0xFF;
	vrts_CMD_STRUCTURE.para[0] = (VENDOR_ID >> 8) & 0xFF;
	vrts_CMD_STRUCTURE.para[1] = STATUS_CMD_SCENE & 0xFF;
	vrts_CMD_STRUCTURE.para[2] = (STATUS_CMD_SCENE >> 8) & 0xFF;
	vrts_CMD_STRUCTURE.para[3] = CURTAIN_CONTROL & 0xFF;
	vrts_CMD_STRUCTURE.para[4] = (CURTAIN_CONTROL >> 8) & 0xFF;
	vrts_CMD_STRUCTURE.para[5] = typeControl;
	if (typeControl == CURTAIN_OPEN_PERCENT) {
		vrts_CMD_STRUCTURE.para[6] = percentOpen;
	}
}

static void CURTAIN_Scene_Set(uint16_t idScene) {
	vrts_CMD_STRUCTURE.opCode[0] = RD_OPCODE_SCENE_SEND & 0xFF;
	vrts_CMD_STRUCTURE.opCode[1] = VENDOR_ID & 0xFF;
	vrts_CMD_STRUCTURE.para[0] = (VENDOR_ID >> 8) & 0xFF;
	vrts_CMD_STRUCTURE.para[1] = STATUS_CMD_SCENE & 0xFF;
	vrts_CMD_STRUCTURE.para[2] = (STATUS_CMD_SCENE >> 8) & 0xFF;
	vrts_CMD_STRUCTURE.para[3] = CURTAIN_SCENE_SET & 0xFF;
	vrts_CMD_STRUCTURE.para[4] = (CURTAIN_SCENE_SET >> 8) & 0xFF;
	vrts_CMD_STRUCTURE.para[5] = idScene & 0xFF;
	vrts_CMD_STRUCTURE.para[6] = (idScene >> 8) & 0xFF;
}

static void CURTAIN_Scene_Del(uint16_t idScene) {
	vrts_CMD_STRUCTURE.opCode[0] = RD_OPCODE_SCENE_SEND & 0xFF;
	vrts_CMD_STRUCTURE.opCode[1] = VENDOR_ID & 0xFF;
	vrts_CMD_STRUCTURE.para[0] = (VENDOR_ID >> 8) & 0xFF;
	vrts_CMD_STRUCTURE.para[1] = STATUS_CMD_SCENE & 0xFF;
	vrts_CMD_STRUCTURE.para[2] = (STATUS_CMD_SCENE >> 8) & 0xFF;
	vrts_CMD_STRUCTURE.para[3] = CURTAIN_SCENE_DEL & 0xFF;
	vrts_CMD_STRUCTURE.para[4] = (CURTAIN_SCENE_DEL >> 8) & 0xFF;
	vrts_CMD_STRUCTURE.para[5] = idScene & 0xFF;
	vrts_CMD_STRUCTURE.para[6] = (idScene >> 8) & 0xFF;
}

static void CURTAIN_Status_Request(){
	vrts_CMD_STRUCTURE.opCode[0] = RD_OPCODE_SCENE_SEND & 0xFF;
	vrts_CMD_STRUCTURE.opCode[1] = VENDOR_ID & 0xFF;
	vrts_CMD_STRUCTURE.para[0] = (VENDOR_ID >> 8) & 0xFF;
	vrts_CMD_STRUCTURE.para[1] = STATUS_CMD_SCENE & 0xFF;
	vrts_CMD_STRUCTURE.para[2] = (STATUS_CMD_SCENE >> 8) & 0xFF;
	vrts_CMD_STRUCTURE.para[3] = CURTAIN_STATUS_RSP & 0xFF;
	vrts_CMD_STRUCTURE.para[4] = (CURTAIN_STATUS_RSP >> 8) & 0xFF;
}

static void CURTAIN_Calib(uint8_t status) {
	vrts_CMD_STRUCTURE.opCode[0] = RD_OPCODE_SCENE_SEND & 0xFF;
	vrts_CMD_STRUCTURE.opCode[1] = VENDOR_ID & 0xFF;
	vrts_CMD_STRUCTURE.para[0] = (VENDOR_ID >> 8) & 0xFF;
	vrts_CMD_STRUCTURE.para[1] = STATUS_CMD_SCENE & 0xFF;
	vrts_CMD_STRUCTURE.para[2] = (STATUS_CMD_SCENE >> 8) & 0xFF;
	vrts_CMD_STRUCTURE.para[3] = CURTAIN_CALIB & 0xFF;
	vrts_CMD_STRUCTURE.para[4] = (CURTAIN_CALIB >> 8) & 0xFF;
	vrts_CMD_STRUCTURE.para[5] = status;
}

static void CURTAIN_Config_Motor(uint8_t typeMotor) {
	vrts_CMD_STRUCTURE.opCode[0] = RD_OPCODE_SCENE_SEND & 0xFF;
	vrts_CMD_STRUCTURE.opCode[1] = VENDOR_ID & 0xFF;
	vrts_CMD_STRUCTURE.para[0] = (VENDOR_ID >> 8) & 0xFF;
	vrts_CMD_STRUCTURE.para[1] = STATUS_CMD_SCENE & 0xFF;
	vrts_CMD_STRUCTURE.para[2] = (STATUS_CMD_SCENE >> 8) & 0xFF;
	vrts_CMD_STRUCTURE.para[3] = CURTAIN_CONFIG_MOTOR & 0xFF;
	vrts_CMD_STRUCTURE.para[4] = (CURTAIN_CONFIG_MOTOR >> 8) & 0xFF;
	vrts_CMD_STRUCTURE.para[5] = typeMotor;
}

static void CURTAIN_InitFramUart() {
	vrts_CMD_STRUCTURE.HCI_CMD_GATEWAY[0] = curtain_cmd & 0xFF;
	vrts_CMD_STRUCTURE.HCI_CMD_GATEWAY[1] = (curtain_cmd >> 8) & 0xFF;
	vrts_CMD_STRUCTURE.opCode00[0] = 0x00;
	vrts_CMD_STRUCTURE.opCode00[1] = 0x00;
	vrts_CMD_STRUCTURE.opCode00[2] = 0x00;
	vrts_CMD_STRUCTURE.opCode00[3] = 0x00;
	vrts_CMD_STRUCTURE.retry_cnt = curtain_parRetry_cnt;
	vrts_CMD_STRUCTURE.rsp_max = curtain_parRsp_Max;
}

#define CURTAIN_TIME_WAIT		500
void CURTAIN_Cmd(curtain_cmd_t typeCmd, uint16_t adr, uint8_t typeControl, uint8_t percent, uint16_t idScene) {
	uint16_t cmdLength = 0;
	CURTAIN_InitFramUart();
	vrts_CMD_STRUCTURE.adr_dst[0] = adr & 0xFF;
	vrts_CMD_STRUCTURE.adr_dst[1] = (adr >> 8) & 0xFF;
	switch (typeCmd) {
	case enum_curtain_control:
		CURTAIN_Control(typeControl, percent);
		cmdLength = CURTAIN_LENGTH_CONTROL;
		break;
	case enum_curtain_scene_set:
		CURTAIN_Scene_Set(idScene);
		cmdLength = CURTAIN_LENGTH_SCENE_SET;
		break;
	case enum_curtain_scene_del:
		CURTAIN_Scene_Del(idScene);
		cmdLength = CURTAIN_LENGTH_SCENE_DEL;
		break;
	case enum_curtain_calib:
		CURTAIN_Calib(typeControl);
		cmdLength = CURTAIN_LENGTH_CALIB;
		break;
	case enum_curtain_status_request:
		CURTAIN_Status_Request();
		cmdLength = CURTAIN_LENGTH_STATUS_REQUEST;
		break;
	case enum_curtain_config_motor:
		CURTAIN_Config_Motor(typeControl);
		cmdLength = CURTAIN_LENGTH_CONFIG_MOTOR;
		break;
	}

	uartSendDev_t vrts_DataUartSend;
	vrts_DataUartSend.length = cmdLength;
	vrts_DataUartSend.dataUart = vrts_CMD_STRUCTURE;
	vrts_DataUartSend.timeWait = CURTAIN_TIME_WAIT;
	pthread_mutex_trylock(&vrpth_SendUart);
	bufferDataUart.push_back(vrts_DataUartSend);
//	head = AddTail(vrts_CMD_STRUCTURE);
	pthread_mutex_unlock(&vrpth_SendUart);
}

static uint8_t CURTAIN_listTypeCmd[4] ={55,54,56,57};
void CURTAIN_RSP_Control(TS_GWIF_IncomingData *data) {
	uint16_t adr = data->Message[1] | (data->Message[2] << 8);
	uint8_t typeControl = data->Message[10];
	uint8_t percent = data->Message[11];

	StringBuffer dataMqtt;
	Writer<StringBuffer> json(dataMqtt);
	json.StartObject();
		json.Key("CMD"); json.String("DEVICE_CONTROL");
		json.Key("DATA");
		json.StartObject();
			json.Key("DEVICE_UNICAST_ID"); json.Int(adr);
			json.Key("PROPERTIES");
			json.StartArray();
				json.StartObject();
					json.Key("ID"); json.Int(CURTAIN_listTypeCmd[typeControl]);
					json.Key("VALUE"); json.Int(percent);
				json.EndObject();
			json.EndArray();
		json.EndObject();
	json.EndObject();

//	cout << dataMqtt.GetString() << endl;
	string s = dataMqtt.GetString();
	char * sendT = new char[s.length()+1];
	strcpy(sendT, s.c_str());
	mqtt_send(mosq,(char*)TP_PUB, (char*)sendT);
	delete sendT;
}

void CURTAIN_RSP_Scene_Set(TS_GWIF_IncomingData *data) {
	uint16_t adr = data->Message[1] | (data->Message[2] << 8);
	uint16_t idScene = data->Message[10] | (data->Message[11] << 8);

	StringBuffer dataMqtt;
	Writer<StringBuffer> json(dataMqtt);
	json.StartObject();
		json.Key("CMD");json.String("CURTAIN_SCENE_SET");
		json.Key("DATA");
		json.StartObject();
			json.Key("DEVICE_UNICAST_ID");json.Int(adr);
			json.Key("SCENEID"); json.Int(idScene);
		json.EndObject();
	json.EndObject();

//	cout << dataMqtt.GetString() << endl;
	string s = dataMqtt.GetString();
	char * sendT = new char[s.length()+1];
	strcpy(sendT, s.c_str());
	mqtt_send(mosq,(char*)TP_PUB, (char*)sendT);
	delete sendT;
}

void CURTAIN_RSP_Scene_Del(TS_GWIF_IncomingData *data) {
	uint16_t adr = data->Message[1] | (data->Message[2] << 8);
	uint16_t idScene = data->Message[10] | (data->Message[11] << 8);

	StringBuffer dataMqtt;
	Writer<StringBuffer> json(dataMqtt);
	json.StartObject();
		json.Key("CMD");json.String("CURTAIN_SCENE_DEL");
		json.Key("DATA");
		json.StartObject();
			json.Key("DEVICE_UNICAST_ID");json.Int(adr);
			json.Key("SCENEID"); json.Int(idScene);
		json.EndObject();
	json.EndObject();

//	cout << dataMqtt.GetString() << endl;
	string s = dataMqtt.GetString();
	char * sendT = new char[s.length()+1];
	strcpy(sendT, s.c_str());
	mqtt_send(mosq,(char*)TP_PUB, (char*)sendT);
	delete sendT;
}

void CURTAIN_RSP_Status_Request(TS_GWIF_IncomingData *data) {
	uint16_t adr = data->Message[1] | (data->Message[2] << 8);
	uint8_t percent = data->Message[11];

	StringBuffer dataMqtt;
	Writer<StringBuffer> json(dataMqtt);
	json.StartObject();
		json.Key("CMD");json.String("CURTAIN_STATUS_REQUEST");
		json.Key("DATA");
		json.StartObject();
			json.Key("DEVICE_UNICAST_ID");json.Int(adr);
			json.Key("PERCENT"); json.Int(percent);
		json.EndObject();
	json.EndObject();

//	cout << dataMqtt.GetString() << endl;
	string s = dataMqtt.GetString();
	char * sendT = new char[s.length()+1];
	strcpy(sendT, s.c_str());
	mqtt_send(mosq,(char*)TP_PUB, (char*)sendT);
	delete sendT;
}

void CURTAIN_RSP_Calib(TS_GWIF_IncomingData *data) {
	uint16_t adr = data->Message[1] | (data->Message[2] << 8);
	uint8_t status  = data->Message[10];

	StringBuffer dataMqtt;
	Writer<StringBuffer> json(dataMqtt);
	json.StartObject();
		json.Key("CMD"); json.String("CURTAIN_CALIB");
		json.Key("DATA");
		json.StartObject();
			json.Key("DEVICE_UNICAST_ID"); json.Int(adr);
			json.Key("TYPE_CONTROL"); json.Int(status);
		json.EndObject();
	json.EndObject();

//	cout << dataMqtt.GetString() << endl;
	string s = dataMqtt.GetString();
	char * sendT = new char[s.length()+1];
	strcpy(sendT, s.c_str());
	mqtt_send(mosq,(char*)TP_PUB, (char*)sendT);
	delete sendT;
}

void CURTAIN_RSP_ConfigMotor(TS_GWIF_IncomingData *data) {
	uint16_t adr = data->Message[1] | (data->Message[2] << 8);
	uint8_t  motor  = data->Message[10];

	StringBuffer dataMqtt;
	Writer<StringBuffer> json(dataMqtt);
	json.StartObject();
		json.Key("CMD"); json.String("CURTAIN_CONFIG_MOTOR");
		json.Key("DATA");
		json.StartObject();
			json.Key("DEVICE_UNICAST_ID"); json.Int(adr);
			json.Key("TYPE_MOTOR"); json.Int(motor);
		json.EndObject();
	json.EndObject();

//	cout << dataMqtt.GetString() << endl;
	string s = dataMqtt.GetString();
	char * sendT = new char[s.length()+1];
	strcpy(sendT, s.c_str());
	mqtt_send(mosq,(char*)TP_PUB, (char*)sendT);
	delete sendT;
}

void CURTAIN_RSP_PressBT( TS_GWIF_IncomingData * data){
	uint16_t adr = data->Message[1] | (data->Message[2] << 8);
	uint8_t status  = data->Message[8];
	uint8_t percent = data->Message[9];

	StringBuffer dataMqtt;
	Writer<StringBuffer> json(dataMqtt);
	json.StartObject();
		json.Key("CMD"); json.String("CURTAIN_PRESS");
		json.Key("DATA");
		json.StartObject();
			json.Key("DEVICE_UNICAST_ID"); json.Int(adr);
			json.Key("TYPE"); json.Int(status);
			json.Key("PERCENT"); json.Int(percent);
		json.EndObject();
	json.EndObject();

//	cout << dataMqtt.GetString() << endl;
	string s = dataMqtt.GetString();
	char * sendT = new char[s.length()+1];
	strcpy(sendT, s.c_str());
	mqtt_send(mosq,(char*)TP_PUB, (char*)sendT);
	delete sendT;
}

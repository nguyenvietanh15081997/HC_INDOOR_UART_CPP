/*
 * RemoteMultiButtons.cpp
 *
 *  Created on: Feb 21, 2022
 *      Author: anh
 */

/*
 * Remote with functions:
 * On/off
 * Dim/Cct
 * Control rgb
 * Save 6 scene of light (HC config)-> call scene direct to light
 * Timer onoff light
 */

#include "RemoteMultiButtons.hpp"

using namespace std;
using namespace rapidjson;

#define REMOTE_MUL_LENGTH_CONFIG_GROUP	19
#define REMOTE_MUL_LENGTH_SCENE_SET		20
#define REMOTE_MUL_LENGTH_SCENE_DEL		19

static uint16_t remoteMul_cmd = HCI_CMD_GATEWAY_CMD;
static uint8_t remoteMul_parRetry_cnt = 0x00;
static uint8_t remoteMul_parRsp_Max = 0x00;

static void RemoteMul_Config_group(uint16_t idGroup) {
	vrts_CMD_STRUCTURE.para[3] = REMOTE_MUL_CONFIG_GROUP & 0xFF;
	vrts_CMD_STRUCTURE.para[4] = (REMOTE_MUL_CONFIG_GROUP >> 8) & 0xFF;
	vrts_CMD_STRUCTURE.para[5] = idGroup & 0xFF;
	vrts_CMD_STRUCTURE.para[6] = (idGroup >> 8) & 0xFF;
}

static void RemoteMul_Scene_Set(uint16_t idScene, uint8_t idButton) {
	vrts_CMD_STRUCTURE.para[3] = REMOTE_MUL_SCENE_SET & 0xFF;
	vrts_CMD_STRUCTURE.para[4] = (REMOTE_MUL_SCENE_SET >> 8) & 0xFF;
	vrts_CMD_STRUCTURE.para[5] = idButton;
	vrts_CMD_STRUCTURE.para[6] = idScene & 0xFF;
	vrts_CMD_STRUCTURE.para[7] = (idScene >> 8) & 0xFF;
}

static void RemoteMul_Scene_Del(uint16_t idScene) {
	vrts_CMD_STRUCTURE.para[3] = REMOTE_MUL_SCENE_DEL & 0xFF;
	vrts_CMD_STRUCTURE.para[4] = (REMOTE_MUL_SCENE_DEL >> 8) & 0xFF;
	vrts_CMD_STRUCTURE.para[5] = idScene & 0xFF;
	vrts_CMD_STRUCTURE.para[6] = (idScene >> 8) & 0xFF;
}

static void RemoteMul_InitFramUart() {
	vrts_CMD_STRUCTURE.HCI_CMD_GATEWAY[0] = remoteMul_cmd & 0xFF;
	vrts_CMD_STRUCTURE.HCI_CMD_GATEWAY[1] = (remoteMul_cmd >> 8) & 0xFF;
	vrts_CMD_STRUCTURE.opCode00[0] = 0x00;
	vrts_CMD_STRUCTURE.opCode00[1] = 0x00;
	vrts_CMD_STRUCTURE.opCode00[2] = 0x00;
	vrts_CMD_STRUCTURE.opCode00[3] = 0x00;
	vrts_CMD_STRUCTURE.retry_cnt = remoteMul_parRetry_cnt;
	vrts_CMD_STRUCTURE.rsp_max = remoteMul_parRsp_Max;
}

#define REMOTE_MUL_TIME_WAIT		500
void RemoteMul_Cmd(remote_Cmd_t typeCmd, uint16_t adr, uint16_t idGroup, uint8_t idButton, uint16_t idScene) {
	uint16_t cmdLength = 0;
	RemoteMul_InitFramUart();
	vrts_CMD_STRUCTURE.adr_dst[0] = adr & 0xFF;
	vrts_CMD_STRUCTURE.adr_dst[1] = (adr >> 8) & 0xFF;
	switch (typeCmd){
	case enum_remoteMul_ConfigGroup:
		RemoteMul_Config_group(idGroup);
		cmdLength = REMOTE_MUL_LENGTH_CONFIG_GROUP;
		break;
	case enum_remoteMul_Scene_Set:
		RemoteMul_Scene_Set(idScene, idButton);
		cmdLength = REMOTE_MUL_LENGTH_SCENE_SET;
		break;
	case enum_remoteMul_Scene_Del:
		RemoteMul_Scene_Del(idScene);
		cmdLength = REMOTE_MUL_LENGTH_SCENE_DEL;
		break;
	}

	uartSendDev_t vrts_DataUartSend;
	vrts_DataUartSend.length = cmdLength;
	vrts_DataUartSend.dataUart = vrts_CMD_STRUCTURE;
	vrts_DataUartSend.timeWait = REMOTE_MUL_TIME_WAIT;
	pthread_mutex_trylock(&vrpth_SendUart);
	bufferDataUart.push_back(vrts_DataUartSend);
//	head = AddTail(vrts_CMD_STRUCTURE);
	pthread_mutex_unlock(&vrpth_SendUart);
}

void RemoteMul_Rsp_ConfigGroup(TS_GWIF_IncomingData *data) {
	uint16_t adr = data->Message[1] | (data->Message[2] << 8);
	uint16_t idGroup = data->Message[8] | (data->Message[9] << 8);

	StringBuffer dataMqtt;
	Writer<StringBuffer> json(dataMqtt);
	json.StartObject();
		json.Key("CMD");json.String("REMOTE_MUL_CONFIG_GROUP");
		json.Key("DATA");
		json.StartObject();
			json.Key("DEVICE_UNICAST_ID");json.Int(adr);
			json.Key("GROUPID"); json.Int(idGroup);
		json.EndObject();
	json.EndObject();

//	cout << dataMqtt.GetString() << endl;
	string s = dataMqtt.GetString();
	char * sendT = new char[s.length()+1];
	strcpy(sendT, s.c_str());
	mqtt_send(mosq,(char*)TP_PUB, (char*)sendT);
	delete sendT;
}

void RemoteMul_Rsp_Scene_Set(TS_GWIF_IncomingData * data) {
	uint16_t adr = data->Message[1] | (data->Message[2] << 8);
	uint8_t buttonId = data->Message[8];
	uint16_t idScene = data->Message[9] | (data->Message[10] << 8);

	StringBuffer dataMqtt;
	Writer<StringBuffer> json(dataMqtt);
	json.StartObject();
		json.Key("CMD");json.String("REMOTE_MUL_SCENE_SET");
		json.Key("DATA");
		json.StartObject();
			json.Key("DEVICE_UNICAST_ID");json.Int(adr);
			json.Key("BUTTONID"); json.Int(buttonId);
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

void RemoteMul_Rsp_Scene_Del(TS_GWIF_IncomingData * data) {
	uint16_t adr = data->Message[1] | (data->Message[2] << 8);
	uint16_t idScene = data->Message[8] | (data->Message[9] << 8);

	StringBuffer dataMqtt;
	Writer<StringBuffer> json(dataMqtt);
	json.StartObject();
		json.Key("CMD");json.String("REMOTE_MUL_SCENE_DEL");
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

/*Rsp opcode 0x52*/
static void RemoteMul_Rsp_OnOffGroup(uint16_t adr, uint16_t idGroup, uint8_t status) {
	StringBuffer dataMqtt;
	Writer<StringBuffer> json(dataMqtt);
	json.StartObject();
		json.Key("CMD");json.String("REMOTE_MUL_");
		json.Key("DATA");
		json.StartObject();
			json.Key("DEVICE_UNICAST_ID");json.Int(adr);
			json.Key("GROUPID"); json.Int(idGroup);
			json.Key("STATUS");json.Int(status);
		json.EndObject();
	json.EndObject();

//	cout << dataMqtt.GetString() << endl;
	string s = dataMqtt.GetString();
	char * sendT = new char[s.length()+1];
	strcpy(sendT, s.c_str());
	mqtt_send(mosq,(char*)TP_PUB, (char*)sendT);
	delete sendT;
}

static void RemoteMul_Rsp_DimCctAll(TS_GWIF_IncomingData * data) {

}

static void RemoteMul_Rsp_DimGroup(TS_GWIF_IncomingData * data) {

}

static void RemoteMul_Rsp_CctGroup(TS_GWIF_IncomingData * data) {

}

static void RemoteMul_Rsp_HslGroup(TS_GWIF_IncomingData * data) {

}

static void RemoteMul_Rsp_Timer(TS_GWIF_IncomingData * data) {

}

string cmdJson[] = { "REMOTE_MUL_ONOFF_GROUP", "REMOTE_MUL_DIM_GROUP",
		"REMOTE_MUL_CCT_GROUP", "REMOTE_MUL_HSL_GROUP", "REMOTE_MUL_TIMER"};

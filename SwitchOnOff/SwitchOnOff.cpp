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

#define SWITCH_LENGTH_CONTROL		23
#define SWITCH_LENGTH_SCENE_SET		23
#define SWITCH_LENGTH_SCENE_DEL		19

static uint16_t switch_cmd = HCI_CMD_GATEWAY_CMD;
static uint8_t switch_parRetry_cnt = 0x00;
static uint8_t switch_parRsp_Max = 0x00;

static uint16_t listTypeSw[4] = { 22001, 22002, 22003, 22004 };
static uint16_t listOpcodeControlSW[4] = { SWITCH_1_CONTROL, SWITCH_2_CONTROL, SWITCH_3_CONTROL, SWITCH_4_CONTROL };
static uint16_t listOpcodeSceneSet[4] = { SWITCH_1_SCENE_SET, SWITCH_2_SCENE_SET, SWITCH_3_SCENE_SET, SWITCH_4_SCENE_SET };
static uint16_t listOpcodeSceneDel[4] = { SWITCH_1_SCENE_DEL, SWITCH_2_SCENE_DEL, SWITCH_3_SCENE_DEL, SWITCH_4_SCENE_DEL };

char IndexType (uint16_t typeSw){
	char indexType = -1;
	for(int i = 0;i<4;i++){
		if(typeSw == listTypeSw[i]){
			indexType = i;
			break;
		}
	}
	return indexType;
}

char IndexOpcode (uint16_t opcode){
	char indexOpcode = -1;
	for(int i = 0;i<4;i++){
		if ((opcode == listOpcodeControlSW[i])
				|| (opcode == listOpcodeSceneSet[i])
				|| (opcode == listOpcodeSceneDel[i])) {
			indexOpcode = i;
			break;
		}
	}
	return indexOpcode;
}

static void Switch_Control(uint16_t typeSwitch, uint16_t adr, uint8_t relayId, uint8_t relayValue){
	vrts_CMD_STRUCTURE.adr_dst[0] = adr & 0xFF;
	vrts_CMD_STRUCTURE.adr_dst[1] = (adr>>8) & 0xFF;
	vrts_CMD_STRUCTURE.opCode[0] = RD_OPCODE_SCENE_SEND & 0xFF;
	vrts_CMD_STRUCTURE.opCode[1] = VENDOR_ID & 0xFF;
	vrts_CMD_STRUCTURE.para[0] = (VENDOR_ID>>8) & 0xFF;
	vrts_CMD_STRUCTURE.para[1] = STATUS_CMD_SCENE & 0xFF;
	vrts_CMD_STRUCTURE.para[2] = (STATUS_CMD_SCENE>>8) & 0xFF;
	vrts_CMD_STRUCTURE.para[3] = listOpcodeControlSW[IndexType(typeSwitch)]  & 0xFF;
	vrts_CMD_STRUCTURE.para[4] = (listOpcodeControlSW[IndexType(typeSwitch)] >>8) & 0xFF;
	vrts_CMD_STRUCTURE.para[5] = relayId & 0xFF;
	vrts_CMD_STRUCTURE.para[6] = relayValue & 0xFF;
}

static void Switch_Scene_Set(uint16_t typeSw, uint16_t adr, uint8_t relay1,
		uint8_t relay2, uint8_t relay3, uint8_t relay4, uint16_t sceneId) {
	vrts_CMD_STRUCTURE.adr_dst[0] = adr & 0xFF;
	vrts_CMD_STRUCTURE.adr_dst[1] = (adr >> 8) & 0xFF;
	vrts_CMD_STRUCTURE.opCode[0] = RD_OPCODE_SCENE_SEND & 0xFF;
	vrts_CMD_STRUCTURE.opCode[1] = VENDOR_ID & 0xFF;
	vrts_CMD_STRUCTURE.para[0] = (VENDOR_ID >> 8) & 0xFF;
	vrts_CMD_STRUCTURE.para[1] = STATUS_CMD_SCENE & 0xFF;
	vrts_CMD_STRUCTURE.para[2] = (STATUS_CMD_SCENE >> 8) & 0xFF;
	vrts_CMD_STRUCTURE.para[3] = listOpcodeSceneSet[IndexType(typeSw)] & 0xFF;
	vrts_CMD_STRUCTURE.para[4] = (listOpcodeSceneSet[IndexType(typeSw)] >> 8) & 0xFF;
	vrts_CMD_STRUCTURE.para[5] = sceneId & 0xFF;
	vrts_CMD_STRUCTURE.para[6] = (sceneId >> 8) & 0xFF;
	vrts_CMD_STRUCTURE.para[7] = relay1 & 0xFF;
	vrts_CMD_STRUCTURE.para[8] = relay2 & 0xFF;
	vrts_CMD_STRUCTURE.para[9] = relay3 & 0xFF;
	vrts_CMD_STRUCTURE.para[10] = relay4 & 0xFF;
}

static void Switch_Scene_Del(uint16_t typeSw, uint16_t adr, uint16_t sceneId){
	vrts_CMD_STRUCTURE.adr_dst[0] = adr & 0xFF;
	vrts_CMD_STRUCTURE.adr_dst[1] = (adr >> 8) & 0xFF;
	vrts_CMD_STRUCTURE.opCode[0] = RD_OPCODE_SCENE_SEND & 0xFF;
	vrts_CMD_STRUCTURE.opCode[1] = VENDOR_ID & 0xFF;
	vrts_CMD_STRUCTURE.para[0] = (VENDOR_ID >> 8) & 0xFF;
	vrts_CMD_STRUCTURE.para[1] = STATUS_CMD_SCENE & 0xFF;
	vrts_CMD_STRUCTURE.para[2] = (STATUS_CMD_SCENE >> 8) & 0xFF;
	vrts_CMD_STRUCTURE.para[3] = listOpcodeSceneDel[IndexType(typeSw)] & 0xFF;
	vrts_CMD_STRUCTURE.para[4] = (listOpcodeSceneDel[IndexType(typeSw)] >> 8) & 0xFF;
	vrts_CMD_STRUCTURE.para[5] = (sceneId) & 0xFF;
	vrts_CMD_STRUCTURE.para[6] = (sceneId >> 8) & 0xFF;
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
void Switch_Send_Uart(switch_enum_cmd typeCmd, uint16_t typeSw, uint16_t adr,
		uint8_t relayId, uint8_t relayValue, uint8_t relay1, uint8_t relay2,
		uint8_t relay3, uint8_t relay4, uint16_t sceneId) {
	uint16_t cmdLength = 0;
	InitFramUart();
	if(typeCmd == switch_enum_control){
		Switch_Control(typeSw, adr, relayId, relayValue);
		cmdLength = SWITCH_LENGTH_CONTROL;
	} else if (typeCmd == switch_enum_addscene) {
		Switch_Scene_Set(typeSw, adr, relay1, relay2, relay3, relay4, sceneId);
		cmdLength = SWITCH_LENGTH_SCENE_SET;
	} else if (typeCmd == switch_enum_delscene) {
		Switch_Scene_Del(typeSw, adr, sceneId);
		cmdLength = SWITCH_LENGTH_SCENE_DEL;
	}

	uartSendDev_t vrts_DataUartSend;
	vrts_DataUartSend.length = cmdLength;
	vrts_DataUartSend.dataUart = vrts_CMD_STRUCTURE;
	vrts_DataUartSend.timeWait = SWITCH_TIME_WAIT;
	pthread_mutex_trylock(&vrpth_SendUart);
	bufferDataUart.push_back(vrts_DataUartSend);
//	head = AddTail(vrts_CMD_STRUCTURE);
	pthread_mutex_unlock(&vrpth_SendUart);
}

void Rsp_Switch_Control(TS_GWIF_IncomingData * data){
	uint16_t adr = data->Message[1] | (data->Message[2] << 8);
	uint16_t opcode = data->Message[8] | (data->Message[9] << 8);
	uint8_t relayId = data->Message[10];
	uint8_t relayValue = data->Message[11];

	StringBuffer dataMqtt;
	Writer<StringBuffer> json(dataMqtt);
	json.StartObject();
		json.Key("CMD");json.String("CONTROL_SWITCH");
		json.Key("TYPE");json.Int(listTypeSw[IndexOpcode(opcode)]);
		json.Key("DATA");
		json.StartObject();
			json.Key("DEVICE_UNICAST_ID");json.Int(adr);
			json.Key("RELAY_ID");json.Int(relayId);
			json.Key("RELAY_STATUS");json.Int(relayValue);
		json.EndObject();
	json.EndObject();

//	cout << dataMqtt.GetString() << endl;
	string s = dataMqtt.GetString();
	char * sendT = new char[s.length()+1];
	strcpy(sendT, s.c_str());
	mqtt_send(mosq,(char*)TP_PUB, (char*)sendT);
	delete sendT;
}

void Rsp_Switch_Scene_Set(TS_GWIF_IncomingData * data){
	uint16_t adr = data->Message[1] | (data->Message[2] << 8);
	uint16_t opcode = data->Message[8] | (data->Message[9] << 8);
	uint16_t sceneId = data->Message[10] | (data->Message[11] << 8);
	uint8_t relay1 = data->Message[12];
	uint8_t relay2 = data->Message[13];
	uint8_t relay3 = data->Message[14];
	uint8_t relay4 = data->Message[15];

	StringBuffer dataMqtt;
	Writer<StringBuffer> json(dataMqtt);
	json.StartObject();
		json.Key("CMD");json.String("ADDSCENE_SWITCH");
		json.Key("TYPE");json.Int(listTypeSw[IndexOpcode(opcode)]);
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
	char * sendT = new char[s.length()+1];
	strcpy(sendT, s.c_str());
	mqtt_send(mosq,(char*)TP_PUB, (char*)sendT);
	delete sendT;
}

void Rsp_Switch_Scene_Del(TS_GWIF_IncomingData * data){
	uint16_t adr = data->Message[1] | (data->Message[2] << 8);
	uint16_t opcode = data->Message[8] | (data->Message[9] << 8);
	uint16_t sceneId = data->Message[10] | (data->Message[11] << 8);

	StringBuffer dataMqtt;
	Writer<StringBuffer> json(dataMqtt);
	json.StartObject();
		json.Key("CMD");json.String("DELSCENE_SWITCH4");
		json.Key("TYPE");json.Int(listTypeSw[IndexOpcode(opcode)]);
		json.Key("DATA");
		json.StartObject();
			json.Key("DEVICE_UNICAST_ID");json.Int(adr);
			json.Key("SCENEID");json.Int(sceneId);
		json.EndObject();
	json.EndObject();

//	cout << dataMqtt.GetString() << endl;
	string s = dataMqtt.GetString();
	char * sendT = new char[s.length()+1];
	strcpy(sendT, s.c_str());
	mqtt_send(mosq,(char*)TP_PUB, (char*)sendT);
	delete sendT;
}

void Rsp_Switch_Status(TS_GWIF_IncomingData * data){
	uint16_t adr = data->Message[1] | (data->Message[2] << 8);
	uint16_t opcode = data->Message[6] | (data->Message[7] << 8);
	uint8_t relayId = data->Message[8];
	uint8_t relayValue = data->Message[9];

	StringBuffer dataMqtt;
	Writer<StringBuffer> json(dataMqtt);
	json.StartObject();
		json.Key("CMD");json.String("SWITCH");
		json.Key("TYPE");json.Int(listTypeSw[IndexOpcode(opcode)]);
		json.Key("DATA");
		json.StartObject();
			json.Key("DEVICE_UNICAST_ID");json.Int(adr);
			json.Key("RELAY_ID");json.Int(relayId);
			json.Key("RELAY_STATUS");json.Int(relayValue);
		json.EndObject();
	json.EndObject();

//	cout << dataMqtt.GetString() << endl;
	string s = dataMqtt.GetString();
	char * sendT = new char[s.length()+1];
	strcpy(sendT, s.c_str());
	mqtt_send(mosq,(char*)TP_PUB, (char*)sendT);
	delete sendT;
}

/*
 * Remote.c
 *
 *  Created on: Oct 29, 2021
 *      Author: anh
 */

#include "Remote.hpp"
#include "../Mqtt/Mqtt.hpp"
#include "../BuildCmdUart/BuildCmdUart.hpp"

using namespace std;
using namespace rapidjson;

#define REMOTE_DC								(0x0002)
#define REMOTE_AC								(0x0003)
#define HEADER_SCENE_REMOTE_AC_SET 				(0x0103)
#define HEADER_SCENE_REMOTE_AC_DEL 				(0x0203)
#define HEADER_SCENE_REMOTE_DC_SET 				(0x0102)
#define HEADER_SCENE_REMOTE_DC_DEL 				(0x0202)

#define BUTTON_MAX			(6)
string arrayButton[BUTTON_MAX] = {"BUTTON_1","BUTTON_2","BUTTON_3","BUTTON_4","BUTTON_5","BUTTON_6"};

void RspRemoteStatus(TS_GWIF_IncomingData *data){
	remotersp_t vrts_RemoteRsp;
	uint16_t adr = data->Message[1] | (data->Message[2] << 8);
	vrts_RemoteRsp.buttonID = data->Message[8];
	vrts_RemoteRsp.modeID	= data->Message[9];
	vrts_RemoteRsp.sceneId 	= data->Message[10] | (data->Message[11] << 8);
	if(vrts_RemoteRsp.sceneId){
		FunctionPer(HCI_CMD_GATEWAY_CMD, CallSence_typedef, 65535,
				NULL8, NULL8, NULL16, NULL16, vrts_RemoteRsp.sceneId,
				NULL16, NULL16, NULL16, NULL16,
				0, 17);
	}
	StringBuffer dataMqtt;
	Writer<StringBuffer> json(dataMqtt);
	json.StartObject();
		json.Key("CMD");json.String("REMOTE");
		json.Key("DATA");
		json.StartObject();
			json.Key("DEVICE_UNICAST_ID");json.Int(adr);
			json.Key("BUTTON_VALUE");json.String(arrayButton[vrts_RemoteRsp.buttonID - 1].c_str());
			json.Key("MODE_VALUE");json.Int(vrts_RemoteRsp.modeID);
			json.Key("SCENEID");json.Int(vrts_RemoteRsp.sceneId);
		json.EndObject();
	json.EndObject();

//	cout << dataMqtt.GetString() << endl;
	string s = dataMqtt.GetString();
	char * sendT = new char[s.length()+1];
	strcpy(sendT, s.c_str());
	mqtt_send(mosq,(char*)TP_PUB, (char*)sendT);
	delete sendT;
}

void RspAddSceneRemote(TS_GWIF_IncomingData *data) {
	remoteRspAddScene_t vrts_RemoteRspAddScene;
	uint16_t adr = data->Message[1] | (data->Message[2] << 8);
	vrts_RemoteRspAddScene.typeRemote = data->Message[8] | (data->Message[9] << 8);
	cout << vrts_RemoteRspAddScene.typeRemote << endl;
	string cmdTypeRM = "";
	if (vrts_RemoteRspAddScene.typeRemote == HEADER_SCENE_REMOTE_DC_SET) {
		cmdTypeRM = "ADDSCENE_REMOTE_DC";
	} else if (vrts_RemoteRspAddScene.typeRemote == HEADER_SCENE_REMOTE_AC_SET) {
		cmdTypeRM = "ADDSCENE_REMOTE_AC";
	}
	cout << cmdTypeRM << endl;
	vrts_RemoteRspAddScene.buttonId = data->Message[10];
	vrts_RemoteRspAddScene.modeId	= data->Message[11];
	vrts_RemoteRspAddScene.sceneId 	= data->Message[12] | (data->Message[13] << 8);

	StringBuffer dataMqtt;
	Writer<StringBuffer> json(dataMqtt);
	json.StartObject();
		json.Key("CMD");json.String(cmdTypeRM.c_str());
		json.Key("DATA");
		json.StartObject();
			json.Key("DEVICE_UNICAST_ID");json.Int(adr);
			json.Key("BUTTONID");json.String(arrayButton[vrts_RemoteRspAddScene.buttonId - 1].c_str());
			json.Key("MODEID");json.Int(vrts_RemoteRspAddScene.modeId);
			json.Key("SCENEID");json.Int(vrts_RemoteRspAddScene.sceneId);
		json.EndObject();
	json.EndObject();

//	cout << dataMqtt.GetString() << endl;
	string s = dataMqtt.GetString();
	char * sendT = new char[s.length()+1];
	strcpy(sendT, s.c_str());
	mqtt_send(mosq,(char*)TP_PUB, (char*)sendT);
	delete sendT;
}

void RspRemoteDelScene(TS_GWIF_IncomingData *data){
	uint16_t adr = data->Message[1] | (data->Message[2] << 8);
	remotrRspDelScene_t vrts_RemoteRspDelScene;
	vrts_RemoteRspDelScene.typeRemote = data->Message[8] | (data->Message[9] << 8);
	string cmdTypeRM = "";
	if(vrts_RemoteRspDelScene.typeRemote == HEADER_SCENE_REMOTE_AC_DEL){
		cmdTypeRM = "DELSCENE_REMOTE_AC";
	}
	else if (vrts_RemoteRspDelScene.typeRemote == HEADER_SCENE_REMOTE_DC_DEL){
		cmdTypeRM = "DELSCENE_REMOTE_DC";
	}
	vrts_RemoteRspDelScene.buttonId = data->Message[10];
	vrts_RemoteRspDelScene.modeId   = data->Message[11];

	StringBuffer dataMqtt;
	Writer<StringBuffer> json(dataMqtt);
	json.StartObject();
		json.Key("CMD");json.String(cmdTypeRM.c_str());
		json.Key("DATA");
		json.StartObject();
			json.Key("DEVICE_UNICAST_ID");json.Int(adr);
			json.Key("BUTTONID");json.String(arrayButton[vrts_RemoteRspDelScene.buttonId - 1].c_str());
			json.Key("MODEID");json.Int(vrts_RemoteRspDelScene.modeId);
		json.EndObject();
	json.EndObject();

//	cout << dataMqtt.GetString() << endl;
	string s = dataMqtt.GetString();
	char * sendT = new char[s.length()+1];
	strcpy(sendT, s.c_str());
	mqtt_send(mosq,(char*)TP_PUB, (char*)sendT);
	delete sendT;
}

void RspPowerRemoteStatus(TS_GWIF_IncomingData *data) {
	uint16_t adr = data->Message[1] | (data->Message[2] << 8);
	remotePowerStatus_t vrts_PowerStatus;
	vrts_PowerStatus.battery = data->Message[9];
	StringBuffer dataMqtt;
	Writer<StringBuffer> json(dataMqtt);
	json.StartObject();
		json.Key("CMD");json.String("POWER_STATUS");
		json.Key("DATA");
		json.StartObject();
			json.Key("DEVICE_UNICAST_ID");json.Int(adr);
			json.Key("POWER_VALUE");json.Int(vrts_PowerStatus.battery);
		json.EndObject();
	json.EndObject();

//	cout << dataMqtt.GetString() << endl;
	string s = dataMqtt.GetString();
	char * sendT = new char[s.length()+1];
	strcpy(sendT, s.c_str());
	mqtt_send(mosq,(char*)TP_PUB, (char*)sendT);
	delete sendT;
}


void RspStatusSwitch4 (TS_GWIF_IncomingData *data){
	uint16_t adr = data->Message[1] | (data->Message[2] << 8);
	uint8_t indexRelay = data->Message[8];
	uint8_t valueRelay = data->Message[9];
	StringBuffer dataMqtt;
	Writer<StringBuffer> json(dataMqtt);
	json.StartObject();
		json.Key("CMD");json.String("SWITCH4");
		json.Key("DATA");
		json.StartObject();
			json.Key("DEVICE_UNICAST_ID");json.Int(adr);
			json.Key("RELAY_ID");json.Int(indexRelay);
			json.Key("RELAY_STATUS");json.Int(valueRelay);
		json.EndObject();
	json.EndObject();

//	cout << dataMqtt.GetString() << endl;
	string s = dataMqtt.GetString();
	char * sendT = new char[s.length()+1];
	strcpy(sendT, s.c_str());
	mqtt_send(mosq,(char*)TP_PUB, (char*)sendT);
	delete sendT;
}
void RspControlSwitch4(TS_GWIF_IncomingData *data){
	uint16_t adr = data->Message[1] | (data->Message[2] << 8);
	uint8_t indexRelay = data->Message[10];
	uint8_t valueRelay = data->Message[11];
	StringBuffer dataMqtt;
	Writer<StringBuffer> json(dataMqtt);
	json.StartObject();
		json.Key("CMD");json.String("SWITCH4");
		json.Key("DATA");
		json.StartObject();
			json.Key("DEVICE_UNICAST_ID");json.Int(adr);
			json.Key("RELAY_ID");json.Int(indexRelay);
			json.Key("RELAY_STATUS");json.Int(valueRelay);
		json.EndObject();
	json.EndObject();

//	cout << dataMqtt.GetString() << endl;
	string s = dataMqtt.GetString();
	char * sendT = new char[s.length()+1];
	strcpy(sendT, s.c_str());
	mqtt_send(mosq,(char*)TP_PUB, (char*)sendT);
	delete sendT;
}

void RspAddSceneSwitch4(TS_GWIF_IncomingData * data){
	uint16_t adr = data->Message[1] | (data->Message[2] << 8);
	uint16_t sceneId = data->Message[10] | (data->Message[11] << 8);
	uint8_t arrayStatusRelay[4] = { data->Message[12], data->Message[13],
			data->Message[14], data->Message[15] };
	string arrayRelay[4] = {"RELAY1","RELAY2","RELAY3","RELAY4"};

	StringBuffer dataMqtt;
	Writer<StringBuffer> json(dataMqtt);
	json.StartObject();
		json.Key("CMD");json.String("ADDSCENE_SWITCH4");
		json.Key("DATA");
		json.StartObject();
			json.Key("DEVICE_UNICAST_ID");json.Int(adr);
			json.Key("SWITCH_STATUS");
			json.StartObject();
			for(int i=0;i<4;i++){
				json.Key(arrayRelay[i].c_str());json.Int(arrayStatusRelay[i]);
			}
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

void RspDelSceneSwitch4(TS_GWIF_IncomingData *data){
	uint16_t adr = data->Message[1] | (data->Message[2] << 8);
	uint16_t sceneId = data->Message[10] | (data->Message[11] << 8);

	StringBuffer dataMqtt;
	Writer<StringBuffer> json(dataMqtt);
	json.StartObject();
		json.Key("CMD");json.String("DELSCENE_SWITCH4");
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













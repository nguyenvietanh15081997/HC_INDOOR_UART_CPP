/*
 * Light.c
 *
 *  Created on: Oct 29, 2021
 *      Author: anh
 */

#include "Light.hpp"
#include "../Mqtt/Mqtt.hpp"
#include "../ProcessUart/Provision.hpp"

using namespace std;
using namespace rapidjson;


static uint8_t Param2PrecentCCT(uint16_t param) {
	return ((param - 800) / 192);
}
static uint8_t Param2PrecentDIM(uint16_t param) {
	return ((param * 100) / 65535);
}
//static uint8_t Param2PercentHSL(uint16_t param) {
//	return ((param * 100) / 65535);
//}
void RspOnoff(TS_GWIF_IncomingData *data) {
	uint16_t adr = data->Message[1] | (data->Message[2] << 8);
	uint16_t length = data->Length[0] | (data->Length[1] << 8);
	int onoffStatus;
	if (length == 9) {
		onoffStatus = data->Message[7];
	} else {
		onoffStatus = data->Message[8];
	}

	StringBuffer dataMqtt;
	Writer<StringBuffer> json(dataMqtt);
	json.StartObject();
		json.Key("CMD");json.String("ONOFF");
		json.Key("DATA");
		json.StartObject();
			json.Key("DEVICE_UNICAST_ID");json.Int(adr);
			json.Key("VALUE_ONOFF");json.Int(onoffStatus);
		json.EndObject();
	json.EndObject();

//	cout << dataMqtt.GetString() << endl;
	string s = dataMqtt.GetString();
	dataSendMqtt_t mqttSend;
	char * sendT = new char[s.length()+1];
	strcpy(sendT, s.c_str());
	memcpy((char*)&mqttSend.dataSendMqtt[0], sendT, s.length() + 1);
	pthread_mutex_lock(&keyBufferSendMqtt);
	ring_push_head(&bufferSendMqtt,(void *) &mqttSend);
	pthread_mutex_unlock(&keyBufferSendMqtt);
	delete sendT;
}

void RspCCT(TS_GWIF_IncomingData *data) {
	uint16_t adr = data->Message[1] | (data->Message[2] << 8);
	int length = data->Length[0] | (data->Length[1] << 8);
	int CCTStatus;
	if (length == 12) {
		CCTStatus = data->Message[7] | (data->Message[8] << 8);
	} else {
		CCTStatus = data->Message[11] | (data->Message[12] << 8);
	}

	StringBuffer dataMqtt;
	Writer<StringBuffer> json(dataMqtt);
	json.StartObject();
		json.Key("CMD");json.String("CCT");
		json.Key("DATA");
		json.StartObject();
			json.Key("DEVICE_UNICAST_ID");json.Int(adr);
			json.Key("VALUE_CCT");json.Int(Param2PrecentCCT(CCTStatus));
		json.EndObject();
	json.EndObject();

//	cout << dataMqtt.GetString() << endl;
	string s = dataMqtt.GetString();
	dataSendMqtt_t mqttSend;
	char * sendT = new char[s.length()+1];
	strcpy(sendT, s.c_str());
	memcpy((char*)&mqttSend.dataSendMqtt[0], sendT, s.length() + 1);
	pthread_mutex_lock(&keyBufferSendMqtt);
	ring_push_head(&bufferSendMqtt,(void *) &mqttSend);
	pthread_mutex_unlock(&keyBufferSendMqtt);
	delete sendT;
}

void RspDIM(TS_GWIF_IncomingData *data) {
	uint16_t adr = data->Message[1] | (data->Message[2] << 8);
	int length = data->Length[0] | (data->Length[1] << 8);
	int DIMStatus;
	if (length == 10) {
		DIMStatus = data->Message[7] | (data->Message[8] << 8);
	} else if (length == 13) {
		DIMStatus = data->Message[9] | (data->Message[10] << 8);
	}

	StringBuffer dataMqtt;
	Writer<StringBuffer> json(dataMqtt);
	json.StartObject();
		json.Key("CMD");json.String("DIM");
		json.Key("DATA");
		json.StartObject();
			json.Key("DEVICE_UNICAST_ID");json.Int(adr);
			json.Key("VALUE_DIM");json.Int(Param2PrecentDIM(DIMStatus));
		json.EndObject();
	json.EndObject();

//	cout << dataMqtt.GetString() << endl;
	string s = dataMqtt.GetString();
	dataSendMqtt_t mqttSend;
	char * sendT = new char[s.length()+1];
	strcpy(sendT, s.c_str());
	memcpy((char*)&mqttSend.dataSendMqtt[0], sendT, s.length() + 1);
	pthread_mutex_lock(&keyBufferSendMqtt);
	ring_push_head(&bufferSendMqtt,(void *) &mqttSend);
	pthread_mutex_unlock(&keyBufferSendMqtt);
	delete sendT;
}

void RspHSL(TS_GWIF_IncomingData *data) {
	uint16_t adr = data->Message[1] | (data->Message[2] << 8);
	int h_value = data->Message[9] | (data->Message[10] << 8);
	int s_value = data->Message[11] | (data->Message[12] << 8);
	int l_value = data->Message[7] | (data->Message[8] << 8);

	StringBuffer dataMqtt;
	Writer<StringBuffer> json(dataMqtt);
	json.StartObject();
		json.Key("CMD");json.String("HSL");
		json.Key("DATA");
		json.StartObject();
			json.Key("DEVICE_UNICAST_ID");json.Int(adr);
			json.Key("VALUE_H");json.Int(h_value);
			json.Key("VALUE_S");json.Int(s_value);
			json.Key("VALUE_L");json.Int(l_value);
		json.EndObject();
	json.EndObject();

//	cout << dataMqtt.GetString() << endl;
	string s = dataMqtt.GetString();
	dataSendMqtt_t mqttSend;
	char * sendT = new char[s.length()+1];
	strcpy(sendT, s.c_str());
	memcpy((char*)&mqttSend.dataSendMqtt[0], sendT, s.length() + 1);
	pthread_mutex_lock(&keyBufferSendMqtt);
	ring_push_head(&bufferSendMqtt,(void *) &mqttSend);
	pthread_mutex_unlock(&keyBufferSendMqtt);
	delete sendT;
}

void RspAddDelGroup(TS_GWIF_IncomingData *data) {
	uint16_t adr = data->Message[1] | (data->Message[2] << 8);
	uint16_t groupId = data->Message[10] | (data->Message[11] << 8);
	string cmd = "";
	if (gvrb_AddGroupLight) {
		cmd = "ADDGROUP";
	} else {
		cmd = "DELGROUP";
	}
	StringBuffer dataMqtt;
	Writer<StringBuffer> json(dataMqtt);
	json.StartObject();
		json.Key("CMD");json.String(cmd.c_str());
		json.Key("DATA");
		json.StartObject();
			json.Key("DEVICE_UNICAST_ID");json.Int(adr);
			json.Key("GROUP_UNICAST_ID");json.Int(groupId);
		json.EndObject();
	json.EndObject();

//	cout << dataMqtt.GetString() << endl;
	string s = dataMqtt.GetString();
	dataSendMqtt_t mqttSend;
	char * sendT = new char[s.length()+1];
	strcpy(sendT, s.c_str());
	memcpy((char*)&mqttSend.dataSendMqtt[0], sendT, s.length() + 1);
	pthread_mutex_lock(&keyBufferSendMqtt);
	ring_push_head(&bufferSendMqtt,(void *) &mqttSend);
	pthread_mutex_unlock(&keyBufferSendMqtt);
	delete sendT;
}

void RspAddDelSceneLight(TS_GWIF_IncomingData *data) {
	uint16_t adr = data->Message[1] | (data->Message[2] << 8);
	uint16_t sceneAddId = data->Message[8] | (data->Message[9] << 8);
	string cmd = "";
	if (sceneAddId == gSceneIdDel) {
		cmd = "ADDSCENE";
	} else {
		cmd = "DELSCENE";
		sceneAddId = gSceneIdDel;
	}
	StringBuffer dataMqtt;
	Writer<StringBuffer> json(dataMqtt);
	json.StartObject();
		json.Key("CMD");json.String(cmd.c_str());
		json.Key("DATA");
		json.StartObject();
			json.Key("DEVICE_UNICAST_ID");json.Int(adr);
			json.Key("SCENEID");json.Int(sceneAddId);
		json.EndObject();
	json.EndObject();

//	cout << dataMqtt.GetString() << endl;
	string s = dataMqtt.GetString();
	dataSendMqtt_t mqttSend;
	char * sendT = new char[s.length()+1];
	strcpy(sendT, s.c_str());
	memcpy((char*)&mqttSend.dataSendMqtt[0], sendT, s.length() + 1);
	pthread_mutex_lock(&keyBufferSendMqtt);
	ring_push_head(&bufferSendMqtt,(void *) &mqttSend);
	pthread_mutex_unlock(&keyBufferSendMqtt);
	delete sendT;
}

void RspCallScene(TS_GWIF_IncomingData *data) {
	uint16_t adr = data->Message[1] | (data->Message[2] << 8);
	int length = data->Length[0] | (data->Length[1] << 8);
	uint16_t sceneId;
	uint8_t srgbId;
	if ((length == 13) || (length == 15)) {
		sceneId = data->Message[9] | (data->Message[10] << 8);
		srgbId = data->Message[13];
	} else {
		sceneId = data->Message[7] | (data->Message[8] << 8);
		srgbId = data->Message[11];
	}
	StringBuffer dataMqtt;
	Writer<StringBuffer> json(dataMqtt);
	if(sceneId){
		json.StartObject();
			json.Key("CMD");json.String("CALLSCENE");
			json.Key("DATA");
			json.StartObject();
				json.Key("DEVICE_UNICAST_ID");json.Int(adr);
				json.Key("SCENEID");json.Int(sceneId);
			json.EndObject();
		json.EndObject();
	}
	else {
		if(srgbId){
			json.StartObject();
				json.Key("CMD");json.String("CALLMODE_RGB");
				json.Key("DATA");
				json.StartObject();
					json.Key("DEVICE_UNICAST_ID");json.Int(adr);
					json.Key("SRGBID");json.Int(srgbId);
				json.EndObject();
			json.EndObject();
		}
	}

//	cout << dataMqtt.GetString() << endl;
	string s = dataMqtt.GetString();
	dataSendMqtt_t mqttSend;
	char * sendT = new char[s.length()+1];
	strcpy(sendT, s.c_str());
	memcpy((char*)&mqttSend.dataSendMqtt[0], sendT, s.length() + 1);
	pthread_mutex_lock(&keyBufferSendMqtt);
	ring_push_head(&bufferSendMqtt,(void *) &mqttSend);
	pthread_mutex_unlock(&keyBufferSendMqtt);
	delete sendT;
}

void RspCallModeRgb(TS_GWIF_IncomingData *data){
	uint16_t adr = data->Message[1] | (data->Message[2] << 8);
	uint8_t modeRgb = data->Message[12];
	if(modeRgb){
		StringBuffer dataMqtt;
		Writer<StringBuffer> json(dataMqtt);
		json.StartObject();
			json.Key("CMD");json.String("CALLMODE_RGB");
			json.Key("DATA");
			json.StartObject();
				json.Key("DEVICE_UNICAST_ID");json.Int(adr);
				json.Key("SRGBID");json.Int(modeRgb);
			json.EndObject();
		json.EndObject();

	//	cout << dataMqtt.GetString() << endl;
		string s = dataMqtt.GetString();
		dataSendMqtt_t mqttSend;
		char * sendT = new char[s.length()+1];
		strcpy(sendT, s.c_str());
		memcpy((char*)&mqttSend.dataSendMqtt[0], sendT, s.length() + 1);
		pthread_mutex_lock(&keyBufferSendMqtt);
		ring_push_head(&bufferSendMqtt,(void *) &mqttSend);
		pthread_mutex_unlock(&keyBufferSendMqtt);
		delete sendT;
	}
}

void RspSaveGw(TS_GWIF_IncomingData *data){
	uint16_t adr = data->Message[1] | (data->Message[2] << 8);
	uint8_t adrGw = data->Message[3] | (data->Message[4] <<8);
	StringBuffer dataMqtt;
	Writer<StringBuffer> json(dataMqtt);
	json.StartObject();
		json.Key("CMD");json.String("SAVEGATEWAY");
		json.Key("DATA");
		json.StartObject();
			json.Key("DEVICE_UNICAST_ID");json.Int(adr);
			json.Key("ADR_GW");json.Int(adrGw);
		json.EndObject();
	json.EndObject();

//	cout << dataMqtt.GetString() << endl;
	string s = dataMqtt.GetString();
	dataSendMqtt_t mqttSend;
	char * sendT = new char[s.length()+1];
	strcpy(sendT, s.c_str());
	memcpy((char*)&mqttSend.dataSendMqtt[0], sendT, s.length() + 1);
	pthread_mutex_lock(&keyBufferSendMqtt);
	ring_push_head(&bufferSendMqtt,(void *) &mqttSend);
	pthread_mutex_unlock(&keyBufferSendMqtt);
	delete sendT;
}

static uint16_t TypeConvert(uint8_t type, uint8_t attribute, uint8_t appli) {
	uint16_t id = 65535;
	if((type != 255) && (attribute != 255) && (appli != 255)){
		id = (type*10000 + attribute*1000 + appli);
	}
	return id;
}
void RspTypeDevice(TS_GWIF_IncomingData *data){
	uint16_t adr = data->Message[1] | (data->Message[2] << 8);
	uint16_t typeDev = TypeConvert(data->Message[10],data->Message[11],data->Message[12]);
	char framVer[5] = {0};
	sprintf((char *)framVer,"%d.%d",data->Message[14],data->Message[15]);

	StringBuffer dataMqtt;
	Writer<StringBuffer> json(dataMqtt);
	json.StartObject();
		json.Key("CMD");json.String("NEW_DEVICE");
		json.Key("DATA");
		json.StartObject();
			json.Key("DEVICE_UNICAST_ID");json.Int(adr);
			json.Key("DEVICE_ID");json.String((char *)PRO_uuid);
			json.Key("DEVICE_TYPE_ID");json.Int(typeDev);
			json.Key("MAC_ADDRESS");json.String((char *)PRO_mac);
			json.Key("FIRMWARE_VERSION");json.String((char *)framVer);
			json.Key("DEVICE_KEY");json.String((char *)PRO_deviceKey);
			json.Key("NET_KEY");json.String((char *)PRO_netKey);
			json.Key("APP_KEY");json.String((char *)PRO_appKey);
		json.EndObject();
	json.EndObject();

//	cout << dataMqtt.GetString() << endl;
	string s = dataMqtt.GetString();
	dataSendMqtt_t mqttSend;
	char * sendT = new char[s.length()+1];
	strcpy(sendT, s.c_str());
	memcpy((char*)&mqttSend.dataSendMqtt[0], sendT, s.length() + 1);
	pthread_mutex_lock(&keyBufferSendMqtt);
	ring_push_head(&bufferSendMqtt,(void *) &mqttSend);
	pthread_mutex_unlock(&keyBufferSendMqtt);
	delete sendT;
}

void RspResetNode (TS_GWIF_IncomingData *data){
	uint16_t adr = data->Message[1] | (data->Message[2] << 8);

	StringBuffer dataMqtt;
	Writer<StringBuffer> json(dataMqtt);
	json.StartObject();
		json.Key("CMD");json.String("RESETNODE");
		json.Key("DATA");
		json.StartObject();
			json.Key("DEVICE_UNICAST_ID");json.Int(adr);
		json.EndObject();
	json.EndObject();

//	cout << dataMqtt.GetString() << endl;
	string s = dataMqtt.GetString();
	dataSendMqtt_t mqttSend;
	char * sendT = new char[s.length()+1];
	strcpy(sendT, s.c_str());
	memcpy((char*)&mqttSend.dataSendMqtt[0], sendT, s.length() + 1);
	pthread_mutex_lock(&keyBufferSendMqtt);
	ring_push_head(&bufferSendMqtt,(void *) &mqttSend);
	pthread_mutex_unlock(&keyBufferSendMqtt);
	delete sendT;
}


/*
 * Light.c
 *
 *  Created on: Oct 29, 2021
 *      Author: anh
 */

#include "Light.hpp"
#include "../ProcessUart/Provision.hpp"
#include "../logging/slog.h"

using namespace std;
using namespace rapidjson;

static uint8_t Param2PrecentCCT(uint16_t param) {
	return ((param - 800) / 192);
}
static uint8_t Param2PrecentDIM(uint16_t param) {
	return ((param * 100) / 65535);
}

#if 0
static uint8_t Param2PercentHSL(uint16_t param) {
	return ((param * 100) / 65535);
}
#endif

void RspTTL(TS_GWIF_IncomingData *data){
	uint16_t adr = data->Message[1] | (data->Message[2] << 8);

	StringBuffer dataMqtt;
	Writer<StringBuffer> json(dataMqtt);
	json.StartObject();
		json.Key("CMD");json.String("UPDATE");
		json.Key("DATA");
		json.StartObject();
			json.Key("DEVICE_UNICAST_ID");json.Int(adr);
		json.EndObject();
	json.EndObject();

	string s = dataMqtt.GetString();
	slog_info("<mqtt>send: %s", s.c_str());
	Data2BufferSendMqtt(s);
}

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

	if (vrte_TypeCmd == typeCmd_Control){
		json.StartObject();
			json.Key("CMD");json.String("ONOFF");
			json.Key("DATA");
			json.StartObject();
				json.Key("DEVICE_UNICAST_ID");json.Int(adr);
				json.Key("VALUE_ONOFF");json.Int(onoffStatus);
			json.EndObject();
		json.EndObject();
	} else if (vrte_TypeCmd == typeCmd_Update){
		json.StartObject();
			json.Key("CMD");json.String("UPDATE");
			json.Key("DATA");
			json.StartObject();
				json.Key("DEVICE_UNICAST_ID");json.Int(adr);
				json.Key("PROPERTIES");
				json.StartArray();
					json.StartObject();
						json.Key("ID");json.Int(PROPERTY_ONOFF);
						json.Key("VALUE");json.Int(onoffStatus);
					json.EndObject();
				json.EndArray();
			json.EndObject();
		json.EndObject();
	}
	string s = dataMqtt.GetString();
	slog_info("<mqtt>send: %s", s.c_str());
	Data2BufferSendMqtt(s);
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
	slog_info("<mqtt>send: %s", s.c_str());
	Data2BufferSendMqtt(s);
}

void RspDIM(TS_GWIF_IncomingData *data) {
	uint16_t adr = data->Message[1] | (data->Message[2] << 8);
	int length = data->Length[0] | (data->Length[1] << 8);
	int DIMStatus;
	if (length == 10) {
		DIMStatus = data->Message[7] | (data->Message[8] << 8);
	} else {
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
	slog_info("<mqtt>send: %s", s.c_str());
	Data2BufferSendMqtt(s);
}

void RspDim_CCT(TS_GWIF_IncomingData *data){
	uint16_t adr = data->Message[1] | (data->Message[2] << 8);
	int dim = data->Message[7] | (data->Message[8] << 8);
	int cct = data->Message[9] | (data->Message[10] << 8);

	StringBuffer dataMqtt;
	Writer<StringBuffer> json(dataMqtt);
	json.StartObject();
		json.Key("CMD");json.String("UPDATE");
		json.Key("DATA");
		json.StartObject();
			json.Key("DEVICE_UNICAST_ID");json.Int(adr);
			json.Key("PROPERTIES");
			json.StartArray();
				json.StartObject();
					json.Key("ID");json.Int(PROPERTY_DIM);
					json.Key("VALUE");json.Int(Param2PrecentDIM(dim));
				json.EndObject();
				json.StartObject();
					json.Key("ID");json.Int(PROPERTY_CCT);
					json.Key("VALUE");json.Int(Param2PrecentCCT(cct));
				json.EndObject();
			json.EndArray();
		json.EndObject();
	json.EndObject();

//	cout << dataMqtt.GetString() << endl;
	string s = dataMqtt.GetString();
	slog_info("<mqtt>send: %s", s.c_str());
	Data2BufferSendMqtt(s);
}

void RspHSL(TS_GWIF_IncomingData *data) {
	uint16_t adr = data->Message[1] | (data->Message[2] << 8);
	int h_value = data->Message[9] | (data->Message[10] << 8);
	int s_value = data->Message[11] | (data->Message[12] << 8);
	int l_value = data->Message[7] | (data->Message[8] << 8);

	StringBuffer dataMqtt;
	Writer<StringBuffer> json(dataMqtt);
	if(vrte_TypeCmd == typeCmd_Control){
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
	} else if (vrte_TypeCmd == typeCmd_Update){
		json.StartObject();
			json.Key("CMD");json.String("UPDATE");
			json.Key("DATA");
			json.StartObject();
				json.Key("DEVICE_UNICAST_ID");json.Int(adr);
				json.Key("PROPERTIES");
				json.StartArray();
					json.StartObject();
						json.Key("ID");json.Int(PROPERTY_H);
						json.Key("VALUE");json.Int(h_value);
					json.EndObject();
					json.StartObject();
						json.Key("ID");json.Int(PROPERTY_S);
						json.Key("VALUE");json.Int(s_value);
					json.EndObject();
					json.StartObject();
						json.Key("ID");json.Int(PROPERTY_L);
						json.Key("VALUE");json.Int(l_value);
					json.EndObject();
				json.EndArray();
			json.EndObject();
		json.EndObject();
	}

//	cout << dataMqtt.GetString() << endl;
	string s = dataMqtt.GetString();
	slog_info("<mqtt>send: %s", s.c_str());
	Data2BufferSendMqtt(s);
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
	slog_info("<mqtt>send: %s", s.c_str());
	Data2BufferSendMqtt(s);
}

static int FindSceneDel(uint16_t adr){
	for(int i = 0; i < MAX_DEV; i++){
		if(adr == g_listAdrScene[i][0] && g_listAdrScene[i][1] != 0){
			return i;
		}
	}
	return -1;
}
void RspAddDelSceneLight(TS_GWIF_IncomingData *data) {
	uint16_t adr = data->Message[1] | (data->Message[2] << 8);
	uint16_t sceneAddId = data->Message[8] | (data->Message[9] << 8);
	string cmd = "";
	if (gvrb_AddSceneLight) {
		cmd = "ADDSCENE";
	} else {
		cmd = "DELSCENE";
		while(pthread_mutex_trylock(&vrpth_DelScene) != 0){};
		int indexScene = FindSceneDel(adr);
		sceneAddId = g_listAdrScene[indexScene][1];
		g_listAdrScene[indexScene][0] = 0;
		g_listAdrScene[indexScene][1] = 0;
		slog_info("Xoa mang");
		for(int m = 0;m < MAX_DEV; m++){
			if (g_listAdrScene[m][0] != 0 && g_listAdrScene[m][1] != 0) {
				slog_print(SLOG_INFO, 1, "<%d>:%d- %d", m, g_listAdrScene[m][0], g_listAdrScene[m][1]);
			}
		}
		pthread_mutex_unlock(&vrpth_DelScene);
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
	slog_info("<mqtt>send: %s", s.c_str());
	Data2BufferSendMqtt(s);
}

void RspCallScene(TS_GWIF_IncomingData *data) {
	uint16_t adr = data->Message[1] | (data->Message[2] << 8);
	int length = data->Length[0] | (data->Length[1] << 8);
	uint16_t sceneId;
	if ((length == 13) || (length == 15)) {
		sceneId = data->Message[9] | (data->Message[10] << 8);
	} else {
		sceneId = data->Message[7] | (data->Message[8] << 8);
	}
	if(sceneId){
		StringBuffer dataMqtt;
		Writer<StringBuffer> json(dataMqtt);

		json.StartObject();
			json.Key("CMD");json.String("CALLSCENE");
			json.Key("DATA");
			json.StartObject();
				json.Key("DEVICE_UNICAST_ID");json.Int(adr);
				json.Key("SCENEID");json.Int(sceneId);
			json.EndObject();
		json.EndObject();
		string s = dataMqtt.GetString();
		slog_info("<mqtt>send: %s", s.c_str());
		Data2BufferSendMqtt(s);
	}
}

#define MODE_CCT			1
#define MODE_RGB			0
#define CALLMODE_RGB        0x1909
void RspCallModeRgb_UpdateLight(TS_GWIF_IncomingData *data){
	StringBuffer dataMqtt;
	Writer<StringBuffer> json(dataMqtt);
	uint16_t adr = data->Message[1] | (data->Message[2] << 8);
	uint16_t dim = data->Message[9] | (data->Message[10] << 8);
	uint16_t cct = data->Message[11] | (data->Message[12] << 8);
	uint16_t l = data->Message[9] | (data->Message[10] << 8);
	uint16_t h = data->Message[11] | (data->Message[12] << 8);
	uint16_t s = data->Message[13] | (data->Message[14] << 8);
	if(data->Message[7] == 2){
		uint8_t status = (data->Message[8] >> 4) & 0x0F;
		uint8_t mode = data->Message[8] & 0x0F;
		json.StartObject();
			json.Key("CMD");json.String("UPDATE");
			json.Key("DATA");
			json.StartObject();
				json.Key("DEVICE_UNICAST_ID");json.Int(adr);
				json.Key("PROPERTIES");
				json.StartArray();
					json.StartObject();
						json.Key("ID"); json.Int(PROPERTY_ONOFF);
						json.Key("VALUE"); json.Int(status);
					json.EndObject();
					if(mode == MODE_CCT){
						json.StartObject();
							json.Key("ID"); json.Int(PROPERTY_CCT);
							json.Key("VALUE"); json.Int(Param2PrecentCCT(cct));
						json.EndObject();
						json.StartObject();
							json.Key("ID"); json.Int(PROPERTY_DIM);
							json.Key("VALUE"); json.Int(Param2PrecentDIM(dim));
						json.EndObject();
					} else if(mode == MODE_RGB){
						json.StartObject();
							json.Key("ID"); json.Int(PROPERTY_H);
							json.Key("VALUE"); json.Int(h);
						json.EndObject();
						json.StartObject();
							json.Key("ID"); json.Int(PROPERTY_S);
							json.Key("VALUE"); json.Int(s);
						json.EndObject();
						json.StartObject();
							json.Key("ID"); json.Int(PROPERTY_L);
							json.Key("VALUE"); json.Int(l);
						json.EndObject();
					}
				json.EndArray();
			json.EndObject();
		json.EndObject();
		string s = dataMqtt.GetString();
		slog_info("<mqtt>send: %s", s.c_str());
		Data2BufferSendMqtt(s);
	} else if ((data->Message[7] == ((CALLMODE_RGB >> 8) & 0xFF))
			&& (data->Message[8] == (CALLMODE_RGB & 0xFF))) {
		uint8_t modeRgb = data->Message[9];
		if ((modeRgb == 1) || (modeRgb == 2) || (modeRgb == 3) || (modeRgb == 4)
				|| (modeRgb == 5) || (modeRgb == 6)) {
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
			slog_info("<mqtt>send: %s", s.c_str());
			Data2BufferSendMqtt(s);
		}
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
	slog_info("<mqtt>send: %s", s.c_str());
	Data2BufferSendMqtt(s);
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
	slog_info("<mqtt>send: %s", s.c_str());
	Data2BufferSendMqtt(s);
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
	slog_info("<mqtt>send: %s", s.c_str());
	Data2BufferSendMqtt(s);
}


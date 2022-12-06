/*
 * SensorLight.c
 */

#include "Sensor.hpp"
#include "../Mqtt/Mqtt.hpp"
#include "../BuildCmdUart/BuildCmdUart.hpp"

using namespace std;
using namespace rapidjson;

#define MASK_15LSB     0x7FFF;

void RspDoorSensorAddScene(TS_GWIF_IncomingData *data) {
	uint16_t adr = data->Message[1] | (data->Message[2] << 8);
	doorRspScene_t vrts_DoorRspScene;
	vrts_DoorRspScene.sceneId = data->Message[10] | (data->Message[11] << 8);
	vrts_DoorRspScene.status  = data->Message[12];

	StringBuffer dataMqtt;
	Writer<StringBuffer> json(dataMqtt);
	json.StartObject();
		json.Key("CMD");json.String("ADDSCENE_DOOR_SENSOR");
		json.Key("DATA");
		json.StartObject();
			json.Key("DEVICE_UNICAST_ID");json.Int(adr);
			json.Key("DOOR_SENSOR");
			json.StartObject();
				json.Key("DOOR");json.Int(vrts_DoorRspScene.status);
			json.EndObject();
			json.Key("SCENEID");json.Int(vrts_DoorRspScene.sceneId);
		json.EndObject();
	json.EndObject();

//	cout << dataMqtt.GetString() << endl;
	string s = dataMqtt.GetString();
	slog_info("<mqtt>send: %s", s.c_str());
	Data2BufferSendMqtt(s);
}

void RspDoorSensorDelScene(TS_GWIF_IncomingData *data) {
	uint16_t adr = data->Message[1] | (data->Message[2] << 8);
	doorRspScene_t vrts_DoorSensor;
	vrts_DoorSensor.sceneId = data->Message[10] | (data->Message[11] << 8);
	StringBuffer dataMqtt;
	Writer<StringBuffer> json(dataMqtt);
	json.StartObject();
		json.Key("CMD");json.String("DELSCENE_DOOR_SENSOR");
		json.Key("DATA");
		json.StartObject();
			json.Key("DEVICE_UNICAST_ID");json.Int(adr);
			json.Key("SCENEID");json.Int(vrts_DoorSensor.sceneId);
		json.EndObject();
	json.EndObject();

//	cout << dataMqtt.GetString() << endl;
	string s = dataMqtt.GetString();
	slog_info("<mqtt>send: %s", s.c_str());
	Data2BufferSendMqtt(s);
}

void RspDoorHangOn(TS_GWIF_IncomingData *data) {
	uint16_t adr = data->Message[1] | (data->Message[2] << 8);
	doorSensorStatus_t vrts_DoorStatusRsp;
	vrts_DoorStatusRsp.hangOn = data->Message[8];

	StringBuffer dataMqtt;
	Writer<StringBuffer> json(dataMqtt);
	json.StartObject();
		json.Key("CMD");json.String("DOOR_SENSOR");
		json.Key("DATA");
		json.StartObject();
			json.Key("DEVICE_UNICAST_ID");json.Int(adr);
			json.Key("HANG_VALUE");json.Int(vrts_DoorStatusRsp.hangOn);
		json.EndObject();
	json.EndObject();

//	cout << dataMqtt.GetString() << endl;
	string s = dataMqtt.GetString();
	slog_info("<mqtt>send: %s", s.c_str());
	Data2BufferSendMqtt(s);
}

void RspDoorStatus(TS_GWIF_IncomingData *data) {
	uint16_t adr = data->Message[1] | (data->Message[2] << 8);
	doorSensorStatus_t vrts_DoorStatusRsp;
	vrts_DoorStatusRsp.status = data->Message[8];

	StringBuffer dataMqtt;
	Writer<StringBuffer> json(dataMqtt);
	json.StartObject();
		json.Key("CMD");json.String("DOOR_SENSOR");
		json.Key("DATA");
		json.StartObject();
			json.Key("DEVICE_UNICAST_ID");json.Int(adr);
			json.Key("DOOR_VALUE");json.Int(vrts_DoorStatusRsp.status);
		json.EndObject();
	json.EndObject();

//	cout << dataMqtt.GetString() << endl;
	string s = dataMqtt.GetString();
	slog_info("<mqtt>send: %s", s.c_str());
	Data2BufferSendMqtt(s);
}

void RspPmSensorTempHum(TS_GWIF_IncomingData *data) {
	uint16_t adr = data->Message[1] | (data->Message[2] << 8);
	pmsensorRsp_Temp_Hum_t vrts_PmRspTempHum;
	vrts_PmRspTempHum.hum = (data->Message[8] << 8) | data->Message[9];
	vrts_PmRspTempHum.temp = (data->Message[12] << 8) | data->Message[13];
	int temp_json = vrts_PmRspTempHum.temp;
	if(data->Message[10] == 0xFF){
		temp_json = (-1) *temp_json;
	}
//	ptempIndoor = temp_json;
//	phumIndoor = vrts_PmRspTempHum.hum;
	StringBuffer dataMqtt;
	Writer<StringBuffer> json(dataMqtt);
	json.StartObject();
		json.Key("CMD");json.String("PM_SENSOR");
		json.Key("DATA");
		json.StartObject();
			json.Key("DEVICE_UNICAST_ID");json.Int(adr);
			json.Key("TEMPERATURE_VALUE");json.Int(temp_json * 10);
			json.Key("HUMIDITY_VALUE");json.Int(vrts_PmRspTempHum.hum * 10);
		json.EndObject();
	json.EndObject();

//	cout << dataMqtt.GetString() << endl;
	string s = dataMqtt.GetString();
	slog_info("<mqtt>send: %s", s.c_str());
	Data2BufferSendMqtt(s);
}

void RspPmSensor(TS_GWIF_IncomingData *data) {
	uint16_t adr = data->Message[1] | (data->Message[2] << 8);
	pmsensorRsp_PM_t vrts_PmRsp;
	vrts_PmRsp.PM_2_5 = (data->Message[8] << 8) | data->Message[9];
	ppm25 = vrts_PmRsp.PM_2_5;
	vrts_PmRsp.PM_10 = (data->Message[10] << 8) | data->Message[11];
	vrts_PmRsp.PM_1_0 = (data->Message[12] << 8) | data->Message[13];
	StringBuffer dataMqtt;
	Writer<StringBuffer> json(dataMqtt);
	json.StartObject();
		json.Key("CMD");json.String("PM_SENSOR");
		json.Key("DATA");
		json.StartObject();
			json.Key("DEVICE_UNICAST_ID");json.Int(adr);
			json.Key("PM2.5_VALUE");json.Int(vrts_PmRsp.PM_2_5);
			json.Key("PM10_VALUE");json.Int(vrts_PmRsp.PM_10);
			json.Key("PM1_VALUE");json.Int(vrts_PmRsp.PM_1_0);
		json.EndObject();
	json.EndObject();

//	cout << dataMqtt.GetString() << endl;
	string s = dataMqtt.GetString();
	slog_info("<mqtt>send: %s", s.c_str());
	Data2BufferSendMqtt(s);
}

void RspTempHumSensor(TS_GWIF_IncomingData *data) {
	uint16_t adr = data->Message[1] | (data->Message[2] << 8);
	tempHumSensorStatus_t vrts_TempHumSensor;
	vrts_TempHumSensor.temp = (((data->Message[8] & 0x7F) << 8 ) | data->Message[9]) & MASK_15LSB;
	vrts_TempHumSensor.hum = (data->Message[10] << 8) | data->Message[11];
	int temp_json = vrts_TempHumSensor.temp;
	if((data->Message[8] & 0x80)){
		temp_json = (-1)* temp_json;
	}
	ptempIndoor = temp_json;
	if(temp_json < 0){
		ptempIndoor = temp_json * (-1) | 0x8000;
	}
	phumIndoor = vrts_TempHumSensor.hum;
	StringBuffer dataMqtt;
	Writer<StringBuffer> json(dataMqtt);
	json.StartObject();
		json.Key("CMD");json.String("TEMPERATURE_HUMIDITY");
		json.Key("DATA");
		json.StartObject();
			json.Key("DEVICE_UNICAST_ID");json.Int(adr);
			json.Key("TEMPERATURE_VALUE");json.Int(temp_json);
			json.Key("HUMIDITY_VALUE");json.Int(vrts_TempHumSensor.hum);
		json.EndObject();
	json.EndObject();

//	cout <<"---> Temp: " << ptempIndoor << endl;
//	cout <<"---> Hum: " << phumIndoor << endl;

//	cout << dataMqtt.GetString() << endl;
	string s = dataMqtt.GetString();
	slog_info("<mqtt>send: %s", s.c_str());
	Data2BufferSendMqtt(s);
}

void RspPirSenSor(TS_GWIF_IncomingData *data) {
	uint16_t adr = data->Message[1] | (data->Message[2] << 8);
	pirsensorRsp_t vrts_PirRsp;
	vrts_PirRsp.pir = data->Message[8] | (data->Message[9] << 8);
	vrts_PirRsp.sceneId = data->Message[10] | (data->Message[11] << 8);

	if(vrts_PirRsp.sceneId){
//		FunctionPer(HCI_CMD_GATEWAY_CMD, CallSence_typedef, 65535,
//				NULL8, NULL8, NULL16, NULL16, vrts_PirRsp.sceneId,
//				NULL16, NULL16, NULL16, NULL16,
//				0, 17);
	}
	StringBuffer dataMqtt,dataMqttLux;
	Writer<StringBuffer> json(dataMqtt);
	Writer<StringBuffer> json1(dataMqttLux);
	json.StartObject();
		json.Key("CMD");json.String("PIR_SENSOR");
		json.Key("DATA");
		json.StartObject();
			json.Key("DEVICE_UNICAST_ID");json.Int(adr);
			json.Key("PIR_VALUE");json.Int(vrts_PirRsp.pir);
			json.Key("SCENEID");json.Int(vrts_PirRsp.sceneId);
		json.EndObject();
	json.EndObject();

	string s = dataMqtt.GetString();
	slog_info("<mqtt>send: %s", s.c_str());
	Data2BufferSendMqtt(s);

	if((data->Length[0] | (data->Length[1] << 8)) > 13)
	{
		uint16_t lux = data->Message[12] | (data->Message[13] << 8);
		json1.StartObject();
			json1.Key("CMD");json1.String("LIGHT_SENSOR");
			json1.Key("DATA");
			json1.StartObject();
				json1.Key("DEVICE_UNICAST_ID");json1.Int(adr);
				json1.Key("LUX_VALUE");json1.Int(lux);
			json1.EndObject();
		json1.EndObject();

		string s1 = dataMqttLux.GetString();
		slog_info("<mqtt>send: %s", s1.c_str());
		Data2BufferSendMqtt(s1);
	}
}

static uint16_t CalculateLux(uint16_t rsp_lux){
	unsigned int lux_LSB = 0;
	unsigned char lux_MSB = 0;
	uint16_t lux_Value = 0;
	unsigned int pow = 1;
	unsigned char i;
	lux_LSB = rsp_lux & 0x0FFF;
	lux_MSB = ((rsp_lux>>12) & 0x0F);
	//Lux_Value = 0.01 * pow(2,Lux_MSB) * Lux_LSB; //don't use
	for(i=0;i<lux_MSB;i++){
		pow=pow*2;
	}
	lux_Value=0.01 * pow * lux_LSB;
	return lux_Value;
}
void RspLightSensor(TS_GWIF_IncomingData *data){
	uint16_t adr = data->Message[1] | (data->Message[2] << 8);
	uint16_t lightStatus = (data->Message[8] << 8) | data->Message[9];
//	CalculateLux(lightStatus);
	StringBuffer dataMqtt;
	Writer<StringBuffer> json(dataMqtt);
	json.StartObject();
		json.Key("CMD");json.String("LIGHT_SENSOR");
		json.Key("DATA");
		json.StartObject();
			json.Key("DEVICE_UNICAST_ID");json.Int(adr);
			json.Key("LUX_VALUE");json.Int(CalculateLux(lightStatus));
		json.EndObject();
	json.EndObject();

//	cout << dataMqtt.GetString() << endl;
	string s = dataMqtt.GetString();
	slog_info("<mqtt>send: %s", s.c_str());
	Data2BufferSendMqtt(s);
}

void RspSmoke(TS_GWIF_IncomingData *data) {
	uint16_t adr = data->Message[1] | (data->Message[2] << 8);
	smokeRsp_t vrts_Smoke;
	vrts_Smoke.smoke = data->Message[8];
	vrts_Smoke.battery = data->Message[9];
	StringBuffer dataMqtt;
	Writer<StringBuffer> json(dataMqtt);
	json.StartObject();
		json.Key("CMD");json.String("SMOKE_SENSOR");
		json.Key("DATA");
		json.StartObject();
			json.Key("DEVICE_UNICAST_ID");json.Int(adr);
			json.Key("SMOKE_VALUE");json.Int(vrts_Smoke.smoke);
			json.Key("LOW_BATTERY");json.Int(vrts_Smoke.battery);
		json.EndObject();
	json.EndObject();

//	cout << dataMqtt.GetString() << endl;
	string s = dataMqtt.GetString();
	slog_info("<mqtt>send: %s", s.c_str());
	Data2BufferSendMqtt(s);
}

void RspPir_LightAddScene(TS_GWIF_IncomingData *data){
	uint16_t adr = data->Message[1] | (data->Message[2] << 8);
	uint16_t sceneId = data->Message[10] | (data->Message[11] << 8);
	RD_Sensor_data_tdef vrts_Pir_Rsp;
	vrts_Pir_Rsp.data = (data->Message[12] << 24) | (data->Message[13] << 16) | (data->Message[14] << 8);
	char type = 0;
	if (vrts_Pir_Rsp.Light_Conditon) {
		type = 2;
	} else {
		type = 1;
	}
	StringBuffer dataMqtt;
	Writer<StringBuffer> json(dataMqtt);
	json.StartObject();
		json.Key("CMD");json.String("ADDSCENE_LIGHT_PIR_SENSOR");
		json.Key("DATA");
		json.StartObject();
			json.Key("DEVICE_UNICAST_ID");json.Int(adr);
			json.Key("TYPE");json.Int(type);
			json.Key("PIR_SENSOR");
			json.StartObject();
				json.Key("PIR");json.Int(vrts_Pir_Rsp.Pir_Conditon);
			json.EndObject();
			if(vrts_Pir_Rsp.Light_Conditon){
				json.Key("LIGHT_SENSOR");
				json.StartObject();
					if(vrts_Pir_Rsp.Light_Conditon == 1){
						json.Key("CONDITION");json.Int(4);
						json.Key("LOW_LUX");json.Int(vrts_Pir_Rsp.Lux_low * 10);
					}
					else if (vrts_Pir_Rsp.Light_Conditon == 2){
						json.Key("CONDITION");json.Int(7);
						json.Key("LOW_LUX");json.Int(vrts_Pir_Rsp.Lux_low * 10);
						json.Key("HIGHT_LUX");json.Int(vrts_Pir_Rsp.Lux_hi * 10);
					}
					else if( vrts_Pir_Rsp.Light_Conditon == 3){
						json.Key("CONDITION");json.Int(4);
						json.Key("LOW_LUX");json.Int(vrts_Pir_Rsp.Lux_low * 10);
					}
				json.EndObject();
			}
		json.Key("SCENEID");json.Int(sceneId);
		json.EndObject();
	json.EndObject();

//	cout << dataMqtt.GetString() << endl;
	string s = dataMqtt.GetString();
	slog_info("<mqtt>send: %s", s.c_str());
	Data2BufferSendMqtt(s);
}

void RspPir_LightDelScene(TS_GWIF_IncomingData *data) {
	uint16_t adr = data->Message[1] | (data->Message[2] << 8);
	pirsensorRsp_t vrts_PirRsp;
	vrts_PirRsp.sceneId = data->Message[10] | (data->Message[11] << 8);
	StringBuffer dataMqtt;
	Writer<StringBuffer> json(dataMqtt);
	json.StartObject();
		json.Key("CMD");json.String("DELSCENE_LIGHT_PIR_SENSOR");
		json.Key("DATA");
		json.StartObject();
			json.Key("DEVICE_UNICAST_ID");json.Int(adr);
			json.Key("SCENEID");json.Int(vrts_PirRsp.sceneId);
		json.EndObject();
	json.EndObject();

//	cout << dataMqtt.GetString() << endl;
	string s = dataMqtt.GetString();
	slog_info("<mqtt>send: %s", s.c_str());
	Data2BufferSendMqtt(s);
}

void RspPirTimeAction(TS_GWIF_IncomingData *data) {
	uint16_t adr = data->Message[1] | (data->Message[2] << 8);
	uint16_t timeAction = data->Message[10] | (data->Message[11] << 8);
	StringBuffer dataMqtt;
	Writer<StringBuffer> json(dataMqtt);
	json.StartObject();
		json.Key("CMD");json.String("TIME_ACTION_PIR");
		json.Key("DATA");
		json.StartObject();
			json.Key("DEVICE_UNICAST_ID");json.Int(adr);
			json.Key("TIME");json.Int(timeAction);
		json.EndObject();
	json.EndObject();

//	cout << dataMqtt.GetString() << endl;
	string s = dataMqtt.GetString();
	char * sendT = new char[s.length()+1];
	slog_info("<mqtt>send: %s", s.c_str());
	Data2BufferSendMqtt(s);
}

void RspPowerStatusSensor(TS_GWIF_IncomingData *data){
	uint16_t adr = data->Message[1] | (data->Message[2] << 8);
	powerStatus_t vrts_Power;
	vrts_Power.power = (data->Message[8] << 8) | data->Message[9];
	StringBuffer dataMqtt;
	Writer<StringBuffer> json(dataMqtt);
	json.StartObject();
		json.Key("CMD");json.String("POWER_STATUS");
		json.Key("DATA");
		json.StartObject();
			json.Key("DEVICE_UNICAST_ID");json.Int(adr);
			json.Key("POWER_VALUE");json.Int(vrts_Power.power);
		json.EndObject();
	json.EndObject();

//	cout << dataMqtt.GetString() << endl;
	string s = dataMqtt.GetString();
	slog_info("<mqtt>send: %s", s.c_str());
	Data2BufferSendMqtt(s);
}

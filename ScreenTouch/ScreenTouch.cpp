#include "ScreenTouch.hpp"

#include "../Mqtt/Mqtt.hpp"
#include "../ProcessUart/OpCode.h"
#include "../BuildCmdUart/BuildCmdUart.hpp"

#define LENGTH_ADDSCENE 			20
#define LENGTH_DELSCENE				19
#define LENGTH_EDIT_ICON			20
#define LENGTH_WEATHER_OUT			20
#define LENGTH_WEATHER_IN			23
#define LENGTH_DATE					22
#define LENGTH_TIME					20
#define LENGTH_DELALLSCENE			23
#define LENGTH_DEFAULT_ONOFF		19

#define MASK_14LSB					0x7FFF
#define MASK_1MSB					0x8000

using namespace std;
using namespace rapidjson;

static uint16_t cmd = HCI_CMD_GATEWAY_CMD;
static uint8_t st_parRetry_cnt = 0x00;
static uint8_t st_parRsp_Max = 0x00;

static void initDataSend()
{
	vrts_CMD_STRUCTURE.HCI_CMD_GATEWAY[0]	= cmd & 0xFF;
	vrts_CMD_STRUCTURE.HCI_CMD_GATEWAY[1]	= (cmd>>8) & 0xFF;
	vrts_CMD_STRUCTURE.opCode00[0] 			= 0x00;
	vrts_CMD_STRUCTURE.opCode00[1]			= 0x00;
	vrts_CMD_STRUCTURE.opCode00[2] 			= 0x00;
	vrts_CMD_STRUCTURE.opCode00[3]			= 0x00;
	vrts_CMD_STRUCTURE.retry_cnt   			= st_parRetry_cnt;
	vrts_CMD_STRUCTURE.rsp_max     			= st_parRsp_Max;
}
static void st_AddScene(uint16_t adrSt,uint16_t idScene, uint8_t idIcon){
	initDataSend();
	vrts_CMD_STRUCTURE.adr_dst[0] = adrSt & 0xFF;
	vrts_CMD_STRUCTURE.adr_dst[1] = (adrSt>>8) & 0xFF;
	vrts_CMD_STRUCTURE.opCode[0] = RD_OPCODE_SCENE_SEND & 0xFF;
	vrts_CMD_STRUCTURE.opCode[1] = VENDOR_ID & 0xFF;
	vrts_CMD_STRUCTURE.para[0] = (VENDOR_ID>>8) & 0xFF;
	vrts_CMD_STRUCTURE.para[1] = STATUS_CMD_SCENE & 0xFF;
	vrts_CMD_STRUCTURE.para[2] = (STATUS_CMD_SCENE>>8) & 0xFF;
	vrts_CMD_STRUCTURE.para[3] = ST_HEADER_ADD_SCENE & 0xFF;
	vrts_CMD_STRUCTURE.para[4] = (ST_HEADER_ADD_SCENE>>8) & 0xFF;
	vrts_CMD_STRUCTURE.para[5] = idScene & 0xFF;
	vrts_CMD_STRUCTURE.para[6] = (idScene>>8) & 0xFF;
	vrts_CMD_STRUCTURE.para[7] = idIcon & 0xFF;
}
static void st_Edit_Icon (uint16_t adrSt, uint16_t idScene, uint8_t idIcon){
	initDataSend();
	vrts_CMD_STRUCTURE.adr_dst[0] = adrSt & 0xFF;
	vrts_CMD_STRUCTURE.adr_dst[1] = (adrSt>>8) & 0xFF;
	vrts_CMD_STRUCTURE.opCode[0] = RD_OPCODE_SCENE_SEND & 0xFF;
	vrts_CMD_STRUCTURE.opCode[1] = VENDOR_ID & 0xFF;
	vrts_CMD_STRUCTURE.para[0] = (VENDOR_ID>>8) & 0xFF;
	vrts_CMD_STRUCTURE.para[1] = STATUS_CMD_SCENE & 0xFF;
	vrts_CMD_STRUCTURE.para[2] = (STATUS_CMD_SCENE>>8) & 0xFF;
	vrts_CMD_STRUCTURE.para[3] = ST_HEADER_EDIT_SCENE_ICON  & 0xFF;
	vrts_CMD_STRUCTURE.para[4] = (ST_HEADER_EDIT_SCENE_ICON >>8) & 0xFF;
	vrts_CMD_STRUCTURE.para[5] = idScene & 0xFF;
	vrts_CMD_STRUCTURE.para[6] = (idScene>>8) & 0xFF;
	vrts_CMD_STRUCTURE.para[7] = idIcon & 0xFF;
}
static void st_Del_Scene(uint16_t adrSt, uint16_t idScene){
	initDataSend();
	vrts_CMD_STRUCTURE.adr_dst[0] = adrSt & 0xFF;
	vrts_CMD_STRUCTURE.adr_dst[1] = (adrSt>>8) & 0xFF;
	vrts_CMD_STRUCTURE.opCode[0] = RD_OPCODE_SCENE_SEND & 0xFF;
	vrts_CMD_STRUCTURE.opCode[1] = VENDOR_ID & 0xFF;
	vrts_CMD_STRUCTURE.para[0] = (VENDOR_ID>>8) & 0xFF;
	vrts_CMD_STRUCTURE.para[1] = STATUS_CMD_SCENE & 0xFF;
	vrts_CMD_STRUCTURE.para[2] = (STATUS_CMD_SCENE>>8) & 0xFF;
	vrts_CMD_STRUCTURE.para[3] = ST_HEADER_DEL_SCENE  & 0xFF;
	vrts_CMD_STRUCTURE.para[4] = (ST_HEADER_DEL_SCENE >>8) & 0xFF;
	vrts_CMD_STRUCTURE.para[5] = idScene & 0xFF;
	vrts_CMD_STRUCTURE.para[6] = (idScene>>8) & 0xFF;
}
static void st_Send_Weather_Outdoor(uint16_t adrSt, uint8_t status, uint16_t temp){
	initDataSend();
	vrts_CMD_STRUCTURE.adr_dst[0] = adrSt & 0xFF;
	vrts_CMD_STRUCTURE.adr_dst[1] = (adrSt>>8) & 0xFF;
	vrts_CMD_STRUCTURE.opCode[0] = RD_OPCODE_SCENE_SEND & 0xFF;
	vrts_CMD_STRUCTURE.opCode[1] = VENDOR_ID & 0xFF;
	vrts_CMD_STRUCTURE.para[0] = (VENDOR_ID>>8) & 0xFF;
	vrts_CMD_STRUCTURE.para[1] = STATUS_CMD_SCENE & 0xFF;
	vrts_CMD_STRUCTURE.para[2] = (STATUS_CMD_SCENE>>8) & 0xFF;
	vrts_CMD_STRUCTURE.para[3] = ST_HEADER_GET_WEATHER_OUTDOOR  & 0xFF;
	vrts_CMD_STRUCTURE.para[4] = (ST_HEADER_GET_WEATHER_OUTDOOR >>8) & 0xFF;
	vrts_CMD_STRUCTURE.para[5] = (temp>>8) & 0xFF;
	vrts_CMD_STRUCTURE.para[6] = (temp) & 0xFF;
	vrts_CMD_STRUCTURE.para[7] = status & 0xFF;
}
static void st_Send_Weather_Indoor(uint16_t adrSt, uint16_t temp, uint16_t hum,uint16_t pm25){
	initDataSend();
	vrts_CMD_STRUCTURE.adr_dst[0] = adrSt & 0xFF;
	vrts_CMD_STRUCTURE.adr_dst[1] = (adrSt>>8) & 0xFF;
	vrts_CMD_STRUCTURE.opCode[0] = RD_OPCODE_SCENE_SEND & 0xFF;
	vrts_CMD_STRUCTURE.opCode[1] = VENDOR_ID & 0xFF;
	vrts_CMD_STRUCTURE.para[0] = (VENDOR_ID>>8) & 0xFF;
	vrts_CMD_STRUCTURE.para[1] = STATUS_CMD_SCENE & 0xFF;
	vrts_CMD_STRUCTURE.para[2] = (STATUS_CMD_SCENE>>8) & 0xFF;
	vrts_CMD_STRUCTURE.para[3] = ST_HEADER_GET_WEATHER_INDOOR  & 0xFF;
	vrts_CMD_STRUCTURE.para[4] = (ST_HEADER_GET_WEATHER_INDOOR >>8) & 0xFF;
	vrts_CMD_STRUCTURE.para[5] = (temp>>8) & 0xFF;
	vrts_CMD_STRUCTURE.para[6] = temp & 0xFF;
	vrts_CMD_STRUCTURE.para[7] = (hum >>8) & 0xFF;
	vrts_CMD_STRUCTURE.para[8] = hum & 0xFF;
	vrts_CMD_STRUCTURE.para[9] = (pm25 >> 8) & 0xFF;
	vrts_CMD_STRUCTURE.para[10] = pm25 & 0xFF;
}
static void st_Send_Date(uint16_t adrSt, uint16_t years, uint8_t month, uint8_t date, uint8_t day){
	initDataSend();
	vrts_CMD_STRUCTURE.adr_dst[0] = adrSt & 0xFF;
	vrts_CMD_STRUCTURE.adr_dst[1] = (adrSt>>8) & 0xFF;
	vrts_CMD_STRUCTURE.opCode[0] = RD_OPCODE_SCENE_SEND & 0xFF;
	vrts_CMD_STRUCTURE.opCode[1] = VENDOR_ID & 0xFF;
	vrts_CMD_STRUCTURE.para[0] = (VENDOR_ID>>8) & 0xFF;
	vrts_CMD_STRUCTURE.para[1] = STATUS_CMD_SCENE & 0xFF;
	vrts_CMD_STRUCTURE.para[2] = (STATUS_CMD_SCENE>>8) & 0xFF;
	vrts_CMD_STRUCTURE.para[3] = ST_HEADER_GET_DATE  & 0xFF;
	vrts_CMD_STRUCTURE.para[4] = (ST_HEADER_GET_DATE >>8) & 0xFF;
	vrts_CMD_STRUCTURE.para[5] = (years >>8) & 0xFF;
	vrts_CMD_STRUCTURE.para[6] = years & 0xFF;
	vrts_CMD_STRUCTURE.para[7] = month & 0xFF;
	vrts_CMD_STRUCTURE.para[8] = date & 0xFF;
	vrts_CMD_STRUCTURE.para[9] = day & 0xFF;
}
static void st_Send_Time(uint16_t adrSt, uint8_t hours, uint8_t minute, uint8_t second){
	initDataSend();
	vrts_CMD_STRUCTURE.adr_dst[0] = adrSt & 0xFF;
	vrts_CMD_STRUCTURE.adr_dst[1] = (adrSt>>8) & 0xFF;
	vrts_CMD_STRUCTURE.opCode[0] = RD_OPCODE_SCENE_SEND & 0xFF;
	vrts_CMD_STRUCTURE.opCode[1] = VENDOR_ID & 0xFF;
	vrts_CMD_STRUCTURE.para[0] = (VENDOR_ID>>8) & 0xFF;
	vrts_CMD_STRUCTURE.para[1] = STATUS_CMD_SCENE & 0xFF;
	vrts_CMD_STRUCTURE.para[2] = (STATUS_CMD_SCENE>>8) & 0xFF;
	vrts_CMD_STRUCTURE.para[3] = ST_HEADER_GET_TIME  & 0xFF;
	vrts_CMD_STRUCTURE.para[4] = (ST_HEADER_GET_TIME >>8) & 0xFF;
	vrts_CMD_STRUCTURE.para[5] = hours & 0xFF;
	vrts_CMD_STRUCTURE.para[6] = minute & 0xFF;
	vrts_CMD_STRUCTURE.para[7] = second & 0xFF;
}

static void st_DelAllScene(uint16_t adrSt){
	initDataSend();
	vrts_CMD_STRUCTURE.adr_dst[0] = adrSt & 0xFF;
	vrts_CMD_STRUCTURE.adr_dst[1] = (adrSt>>8) & 0xFF;
	vrts_CMD_STRUCTURE.opCode[0] = RD_OPCODE_SCENE_SEND & 0xFF;
	vrts_CMD_STRUCTURE.opCode[1] = VENDOR_ID & 0xFF;
	vrts_CMD_STRUCTURE.para[0] = (VENDOR_ID>>8) & 0xFF;
	vrts_CMD_STRUCTURE.para[1] = STATUS_CMD_SCENE & 0xFF;
	vrts_CMD_STRUCTURE.para[2] = (STATUS_CMD_SCENE>>8) & 0xFF;
	vrts_CMD_STRUCTURE.para[3] = ST_HEADER_DELALL_SCENE  & 0xFF;
	vrts_CMD_STRUCTURE.para[4] = (ST_HEADER_DELALL_SCENE >>8) & 0xFF;
	vrts_CMD_STRUCTURE.para[5] = 0x00;
	vrts_CMD_STRUCTURE.para[6] = 0x01;
	vrts_CMD_STRUCTURE.para[7] = 0x02;
	vrts_CMD_STRUCTURE.para[8] = 0x03;
	vrts_CMD_STRUCTURE.para[9] = 0x04;
	vrts_CMD_STRUCTURE.para[10] = 0x05;
}

static void st_DefaultOnOff(uint16_t adrSt,uint16_t groupId){
	initDataSend();
	vrts_CMD_STRUCTURE.adr_dst[0] = adrSt & 0xFF;
	vrts_CMD_STRUCTURE.adr_dst[1] = (adrSt>>8) & 0xFF;
	vrts_CMD_STRUCTURE.opCode[0] = RD_OPCODE_SCENE_SEND & 0xFF;
	vrts_CMD_STRUCTURE.opCode[1] = VENDOR_ID & 0xFF;
	vrts_CMD_STRUCTURE.para[0] = (VENDOR_ID>>8) & 0xFF;
	vrts_CMD_STRUCTURE.para[1] = STATUS_CMD_SCENE & 0xFF;
	vrts_CMD_STRUCTURE.para[2] = (STATUS_CMD_SCENE>>8) & 0xFF;
	vrts_CMD_STRUCTURE.para[3] = ST_HEADER_DEFAULT_ONOFF  & 0xFF;
	vrts_CMD_STRUCTURE.para[4] = (ST_HEADER_DEFAULT_ONOFF >>8) & 0xFF;
	vrts_CMD_STRUCTURE.para[5] = (groupId >> 8) & 0xFF;
	vrts_CMD_STRUCTURE.para[6] = groupId & 0xFF;
}

#define SCREENTOUCH_TIME_WAIT			1000
void SendData2ScreenTouch(tpd_enum_st data,uint16_t adr, uint16_t sceneId, uint8_t iconId,
		uint8_t statusWeather, uint16_t temp, uint16_t hum,uint16_t pm25, uint16_t years,
		uint8_t month, uint8_t date, uint8_t day, uint8_t hours, uint8_t minute,
		uint8_t second) {
	uint16_t cmdLength=0;
	if(data == st_enum_addscene){
		st_AddScene(adr,sceneId,iconId);
		cmdLength = LENGTH_ADDSCENE;
	}
	else if (data == st_enum_delscene){
		st_Del_Scene(adr,sceneId);
		cmdLength = LENGTH_DELSCENE;
	}
	else if (data == st_enum_editIcon){
		st_Edit_Icon(adr,sceneId,iconId);
		cmdLength = LENGTH_EDIT_ICON;
	}
	else if (data == st_enum_weatherOutdoor){
		st_Send_Weather_Outdoor(adr,statusWeather,temp);
		cmdLength = LENGTH_WEATHER_OUT;
	}
	else if (data == st_enum_weatherIndoor){
		st_Send_Weather_Indoor(adr,temp,hum,pm25);
		cmdLength = LENGTH_WEATHER_IN;
	}
	else if (data == st_enum_getDate){
		st_Send_Date(adr,years,month,date,day);
		cmdLength = LENGTH_DATE;
	}
	else if (data == st_enum_getTime){
		st_Send_Time(adr,hours,minute,second);
		cmdLength = LENGTH_TIME;
	}
	else if (data == st_enum_DelAllScene){
		st_DelAllScene(adr);
		cmdLength = LENGTH_DELALLSCENE;
	}
	else if (data == st_enum_DefaultOnOff){
		st_DefaultOnOff(adr,sceneId);
		cmdLength = LENGTH_DEFAULT_ONOFF;
	}
	uartSendDev_t vrts_DataUartSend;
	vrts_DataUartSend.length = cmdLength;
	vrts_DataUartSend.dataUart = vrts_CMD_STRUCTURE;
	vrts_DataUartSend.timeWait = SCREENTOUCH_TIME_WAIT;
	while(pthread_mutex_trylock(&vrpth_SendUart) != 0){};
	bufferDataUart.push_back(vrts_DataUartSend);
//	head = AddTail(vrts_CMD_STRUCTURE);
	pthread_mutex_unlock(&vrpth_SendUart);
}


/* Proccess Rsp*/
void RspScreenTouchStatus(TS_GWIF_IncomingData * data){
	uint16_t adr = data->Message[1] | (data->Message[2] << 8);
	ts_Status_St vrts_Status;
	vrts_Status.sceneId = data->Message[9] | (data->Message[10] << 8);
	if(vrts_Status.sceneId){
//		FunctionPer(HCI_CMD_GATEWAY_CMD, CallSence_typedef, 65535,
//				NULL8, NULL8, NULL16, NULL16, vrts_Status.sceneId,
//				NULL16, NULL16, NULL16, NULL16,
//				0, 17);
	}
	StringBuffer dataMqtt;
	Writer<StringBuffer> json(dataMqtt);
	json.StartObject();
		json.Key("CMD");json.String("SCREEN_TOUCH");
		json.Key("DATA");
		json.StartObject();
			json.Key("DEVICE_UNICAST_ID");json.Int(adr);
			json.Key("SCENEID");json.Int(vrts_Status.sceneId);
		json.EndObject();
	json.EndObject();

//	cout << dataMqtt.GetString() << endl;
	string s = dataMqtt.GetString();
	slog_info("<mqtt>send: %s", s.c_str());
	Data2BufferSendMqtt(s);

}
void RspScreenTouchAddScene(TS_GWIF_IncomingData *data){
	uint16_t adr = data->Message[1] | (data->Message[2] << 8);
	ts_Scene_ST vrts_Scene;
	vrts_Scene.idScene = data->Message[10] | (data->Message[11] << 8);
	vrts_Scene.idIconScene = data->Message[12];

	StringBuffer dataMqtt;
	Writer<StringBuffer> json(dataMqtt);
	json.StartObject();
		json.Key("CMD");json.String("ADDSCENE_SCREEN_TOUCH");
		json.Key("DATA");
		json.StartObject();
			json.Key("DEVICE_UNICAST_ID");json.Int(adr);
			json.Key("SCENEID");json.Int(vrts_Scene.idScene);
			json.Key("ICONID");json.Int(vrts_Scene.idIconScene);
		json.EndObject();
	json.EndObject();

//	cout << dataMqtt.GetString() << endl;
	string s = dataMqtt.GetString();
	slog_info("<mqtt>send: %s", s.c_str());
	Data2BufferSendMqtt(s);
}

void RspScreenTouchEditScene(TS_GWIF_IncomingData *data){
	uint16_t adr = data->Message[1] | (data->Message[2] << 8);
	ts_Scene_ST vrts_Scene;
	vrts_Scene.idScene = data->Message[10] | (data->Message[11] << 8);
	vrts_Scene.idIconScene = data->Message[12];

	StringBuffer dataMqtt;
	Writer<StringBuffer> json(dataMqtt);
	json.StartObject();
		json.Key("CMD");json.String("EDITICON_SCREEN_TOUCH");
		json.Key("DATA");
		json.StartObject();
			json.Key("DEVICE_UNICAST_ID");json.Int(adr);
			json.Key("SCENEID");json.Int(vrts_Scene.idScene);
			json.Key("ICONID");json.Int(vrts_Scene.idIconScene);
		json.EndObject();
	json.EndObject();

//	cout << dataMqtt.GetString() << endl;
	string s = dataMqtt.GetString();
	slog_info("<mqtt>send: %s", s.c_str());
	Data2BufferSendMqtt(s);
}

void RspScreenTouchDelScene(TS_GWIF_IncomingData *data){
	uint16_t adr = data->Message[1] | (data->Message[2] << 8);
	ts_Scene_ST vrts_Scene;
	vrts_Scene.idScene = data->Message[10] | (data->Message[11] << 8);

	StringBuffer dataMqtt;
	Writer<StringBuffer> json(dataMqtt);
	json.StartObject();
		json.Key("CMD");json.String("DELSCENE_SCREEN_TOUCH");
		json.Key("DATA");
		json.StartObject();
			json.Key("DEVICE_UNICAST_ID");json.Int(adr);
			json.Key("SCENEID");json.Int(vrts_Scene.idScene);
		json.EndObject();
	json.EndObject();

//	cout << dataMqtt.GetString() << endl;
	string s = dataMqtt.GetString();
	slog_info("<mqtt>send: %s", s.c_str());
	Data2BufferSendMqtt(s);
}

void RspScreenTouchSetWeatherOut(TS_GWIF_IncomingData *data) {
	uint16_t adr = data->Message[1] | (data->Message[2] << 8);
	ts_weatherOut vrts_WeatherOut;
	vrts_WeatherOut.status = data->Message[12];
	vrts_WeatherOut.temp = ((data->Message[10] << 8) | data->Message[11]) & MASK_14LSB;
	int temp_json = vrts_WeatherOut.temp;
	uint16_t check = ((data->Message[10] << 8) | data->Message[11]) & MASK_1MSB;
	if(check){
		temp_json = (-1) * temp_json;
	}
	StringBuffer dataMqtt;
	Writer<StringBuffer> json(dataMqtt);
	json.StartObject();
		json.Key("CMD");json.String("WEATHER_OUT_SCREEN_TOUCH");
		json.Key("DATA");
		json.StartObject();
			json.Key("DEVICE_UNICAST_ID");json.Int(adr);
			json.Key("STATUS");json.Int(vrts_WeatherOut.status);
			json.Key("TEMPERATURE");json.Int(temp_json);
		json.EndObject();
	json.EndObject();

//	cout << dataMqtt.GetString() << endl;
	string s = dataMqtt.GetString();
	slog_info("<mqtt>send: %s", s.c_str());
	Data2BufferSendMqtt(s);
}

void RspScreenTouchWeatherIndoor(TS_GWIF_IncomingData *data) {
	uint16_t adr = data->Message[1] | (data->Message[2] << 8);
	ts_weatherIn vrts_WeatherIndoor;
	vrts_WeatherIndoor.temp = ((data->Message[10] << 8 ) | data->Message[11]) & MASK_14LSB;
	int temp_json = vrts_WeatherIndoor.temp;
	uint16_t check = ((data->Message[10] << 8) | data->Message[11]) & MASK_1MSB;
	if(check){
		temp_json = (-1)* temp_json;
	}
	vrts_WeatherIndoor.hum = (data->Message[12] << 8 ) | data->Message[13];
	vrts_WeatherIndoor.pm25 = (data->Message[14] << 8) | data->Message[15];

	StringBuffer dataMqtt;
	Writer<StringBuffer> json(dataMqtt);
	json.StartObject();
		json.Key("CMD");json.String("WEATHER_IN_SCREEN_TOUCH");
		json.Key("DATA");
		json.StartObject();
			json.Key("DEVICE_UNICAST_ID");json.Int(adr);
			json.Key("TEMPERATURE");json.Int(temp_json);
			json.Key("HUMIDITY");json.Int(vrts_WeatherIndoor.hum);
			json.Key("PM2.5");json.Int(vrts_WeatherIndoor.pm25);
		json.EndObject();
	json.EndObject();

//	cout << dataMqtt.GetString() << endl;
	string s = dataMqtt.GetString();
	slog_info("<mqtt>send: %s", s.c_str());
	Data2BufferSendMqtt(s);
}

void RspScreenTouchTime(TS_GWIF_IncomingData *data) {
	uint16_t adr = data->Message[1] | (data->Message[2] << 8);
	ts_time vrts_Time;
	vrts_Time.hours = data->Message[10];
	vrts_Time.munite = data->Message[11];
	vrts_Time.second = data->Message[12];

	StringBuffer dataMqtt;
	Writer<StringBuffer> json(dataMqtt);
	json.StartObject();
		json.Key("CMD");json.String("TIME_SCREEN_TOUCH");
		json.Key("DATA");
		json.StartObject();
			json.Key("DEVICE_UNICAST_ID");json.Int(adr);
			json.Key("HOUR");json.Int(vrts_Time.hours);
			json.Key("MINUTE");json.Int(vrts_Time.munite);
			json.Key("SECOND");json.Int(vrts_Time.second);
		json.EndObject();
	json.EndObject();

//	cout << dataMqtt.GetString() << endl;
	string s = dataMqtt.GetString();
	slog_info("<mqtt>send: %s", s.c_str());
	Data2BufferSendMqtt(s);
}

void RspScreenTouchDate(TS_GWIF_IncomingData *data) {
	uint16_t adr = data->Message[1] | (data->Message[2] << 8);
	ts_date vrts_Date;
	vrts_Date.years 	= data->Message[10] | (data->Message[11] << 8);
	vrts_Date.month 	= data->Message[12];
	vrts_Date.date_t  	= data->Message[13];
	vrts_Date.day   	= data->Message[14];

	StringBuffer dataMqtt;
	Writer<StringBuffer> json(dataMqtt);
	json.StartObject();
		json.Key("CMD");json.String("TIME_SCREEN_TOUCH");
		json.Key("DATA");
		json.StartObject();
			json.Key("DEVICE_UNICAST_ID");json.Int(adr);
			json.Key("YEAR");json.Int(vrts_Date.years);
			json.Key("MONTH");json.Int(vrts_Date.month);
			json.Key("DATE");json.Int(vrts_Date.date_t);
			json.Key("DAY");json.Int(vrts_Date.day);
		json.EndObject();
	json.EndObject();

//	cout << dataMqtt.GetString() << endl;
	string s = dataMqtt.GetString();
	slog_info("<mqtt>send: %s", s.c_str());
	Data2BufferSendMqtt(s);
}

void RspScreenTouchDelAllScene(TS_GWIF_IncomingData * data){
	uint16_t adr = data->Message[1] | (data->Message[2] << 8);
	StringBuffer dataMqtt;
	Writer<StringBuffer> json(dataMqtt);
	json.StartObject();
		json.Key("CMD");json.String("DELALL_SCENE_SCREENTOUCH");
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

void RspScreenTouchDefaultOnOff(TS_GWIF_IncomingData * data){
	uint16_t adr = data->Message[1] | (data->Message[2] << 8);
	uint16_t groupId = (data->Message[10] << 8 ) | data->Message[11];
	StringBuffer dataMqtt;
	Writer<StringBuffer> json(dataMqtt);
	json.StartObject();
		json.Key("CMD");json.String("DEFAULT_ONOFF_SCREENTOUCH");
		json.Key("DATA");
		json.StartObject();
			json.Key("DEVICE_UNICAST_ID");json.Int(adr);
			json.Key("GROUPID");json.Int(groupId);
		json.EndObject();
	json.EndObject();

//	cout << dataMqtt.GetString() << endl;
	string s = dataMqtt.GetString();
	slog_info("<mqtt>send: %s", s.c_str());
	Data2BufferSendMqtt(s);
}

void RspScreenTouchStatusOnOffGroup(TS_GWIF_IncomingData *data){
	uint16_t adr = data->Message[1] | (data->Message[2] << 8);
	uint16_t groupId = (data->Message[8] << 8 ) | data->Message[9];
	uint8_t status = data->Message[10];
	StringBuffer dataMqtt;
	Writer<StringBuffer> json(dataMqtt);
	json.StartObject();
		json.Key("CMD");json.String("ADJUST_GROUP");
		json.Key("DATA");
		json.StartObject();
			json.Key("DEVICE_UNICAST_ID");json.Int(adr);
			json.Key("GROUPID");json.Int(groupId);
			json.Key("PROPERTIES");
			json.StartArray();
				json.StartObject();
					json.Key("ID");json.Int(PROPERTY_ONOFF);
					json.Key("VALUE"); json.Int(status);
				json.EndObject();
			json.EndArray();
		json.EndObject();
	json.EndObject();

//	cout << dataMqtt.GetString() << endl;
	string s = dataMqtt.GetString();
	slog_info("<mqtt>send: %s", s.c_str());
	Data2BufferSendMqtt(s);
}

#define ST_ADJUST_CCT	1
#define ST_ADJUST_DIM	0
#define ST_ADJUST_HSL	2
void RspScreenTouchAdjust(TS_GWIF_IncomingData * data){
	uint16_t adr = data->Message[1] | (data->Message[2] << 8);
	uint16_t groupId = (data->Message[8]  << 8) | data->Message[9];
	uint8_t typeAdjust = data->Message[10];
	StringBuffer dataMqtt;
	Writer<StringBuffer> json(dataMqtt);
	json.StartObject();
		json.Key("CMD");json.String("ADJUST_GROUP");
		json.Key("DATA");
		json.StartObject();
			json.Key("DEVICE_UNICAST_ID");json.Int(adr);
			json.Key("GROUPID");json.Int(groupId);
			json.Key("PROPERTIES");
			json.StartArray();
				if(typeAdjust == ST_ADJUST_DIM){
					json.StartObject();
					json.Key("ID"); json.Int(PROPERTY_DIM);
					json.Key("VALUE"); json.Int(data->Message[11]);
					json.EndObject();
				} else if(typeAdjust == ST_ADJUST_CCT){
					json.StartObject();
					json.Key("ID"); json.Int(PROPERTY_CCT);
					json.Key("VALUE"); json.Int(data->Message[11]);
					json.EndObject();
				} else if(typeAdjust == ST_ADJUST_HSL){
					json.StartObject();
					json.Key("ID"); json.Int(PROPERTY_H);
					json.Key("VALUE"); json.Int(data->Message[11] | (data->Message[12] << 8));
					json.EndObject();
					json.StartObject();
					json.Key("ID"); json.Int(PROPERTY_S);
					json.Key("VALUE"); json.Int(data->Message[13] | (data->Message[14] << 8));
					json.EndObject();
					json.StartObject();
					json.Key("ID"); json.Int(PROPERTY_L);
					json.Key("VALUE"); json.Int(data->Message[15] | (data->Message[16] << 8));
					json.EndObject();
				}
			json.EndArray();
		json.EndObject();
	json.EndObject();

//	cout << dataMqtt.GetString() << endl;
	string s = dataMqtt.GetString();
	slog_info("<mqtt>send: %s", s.c_str());
	Data2BufferSendMqtt(s);
}

void RequestTime(TS_GWIF_IncomingData *data) {
	uint16_t adr = data->Message[1] | (data->Message[2] << 8);
	StringBuffer dataMqtt;
	Writer<StringBuffer> json(dataMqtt);
	json.StartObject();
		json.Key("CMD");json.String("SCREEN_TOUCH_REQUEST_TIME");
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

void RequestTempHum(TS_GWIF_IncomingData *data) {
	uint16_t adr = data->Message[1] | (data->Message[2] << 8);
	StringBuffer dataMqtt;
	Writer<StringBuffer> json(dataMqtt);
	json.StartObject();
		json.Key("CMD");json.String("SCREEN_TOUCH_REQUEST_TEMP_HUM");
		json.Key("DATA");
		json.StartObject();
			json.Key("DEVICE_UNICAST_ID");json.Int(adr);
		json.EndObject();
	json.EndObject();

//	cout << dataMqtt.GetString() << endl;
	string s = dataMqtt.GetString();
	slog_info("<mqtt>send: %s", s.c_str());
	Data2BufferSendMqtt(s);
//	int temp = ptempIndoor/10;
//	int hum = phumIndoor/10;
//	if((ptempIndoor % 10) >=5 ){
//		temp++;
//	}
//	if((phumIndoor % 10) >= 5){
//		hum++;
//	}

//	cout <<"---> Temp: " << ptempIndoor << endl;
//	cout <<"---> Hum: " << phumIndoor << endl;

//	SendData2ScreeTouch(st_enum_weatherIndoor, adr, 0, 0, 0, temp,
//			hum, ppm25, 0, 0, 0, 0, 0, 0, 0);
}





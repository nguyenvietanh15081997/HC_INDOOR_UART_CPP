#ifndef SCREENTOUCH_HPP_
#define SCREENTOUCH_HPP_

#include "../Include/Include.hpp"

#define ST_HEADER_ADD_SCENE 			0x010A
#define ST_HEADER_DEL_SCENE 			0x020A
#define ST_HEADER_EDIT_SCENE_NAME_ICON 	0x040A
#define ST_HEADER_EDIT_SCENE_ICON 		0x070A
#define ST_HEADER_GET_WEATHER_OUTDOOR 	0x050A
#define ST_HEADER_GET_WEATHER_INDOOR 	0x030A
#define ST_HEADER_GET_DATE			 	0x080A
#define ST_HEADER_GET_TIME			 	0x090A
#define ST_HEADER_DELALL_SCENE			0x0A0A
#define ST_HEADER_DEFAULT_ONOFF			0x0B0A
#define ST_HEADER_REQUEST_TIME			0xF00A
#define ST_HEADER_REQUEST_TEMP_HUM	    0xF10A
#define ST_HEADER_ONOFF_GROUP			0x440A

typedef struct Status_ST{
	uint16_t 	header;
	uint16_t 	sceneId;
	uint8_t 	buttonId;
}ts_Status_St;

typedef struct Scene_ST{
	uint8_t header[2];
	uint16_t idScene;
	uint8_t idIconScene;
}ts_Scene_ST;
typedef struct weatherOut{
	uint8_t header[2];
	uint16_t temp;
	uint8_t status;
}ts_weatherOut;
typedef struct weatherIn{
	uint8_t header[2];
	uint16_t temp;
	uint16_t hum;
	uint16_t pm25;
}ts_weatherIn;
typedef struct date{
	uint8_t header[2];
	uint16_t years;
	uint8_t month;
	uint8_t date_t;
	uint8_t day;
}ts_date;
typedef struct time{
	uint8_t header[2];
	uint8_t hours;
	uint8_t munite;
	uint8_t second;
}ts_time;

typedef enum{
	st_enum_addscene,
	st_enum_delscene,
	st_enum_editName_Icon,
	st_enum_editIcon,
	st_enum_weatherOutdoor,
	st_enum_weatherIndoor,
	st_enum_getTime,
	st_enum_getDate,
	st_enum_DelAllScene,
	st_enum_DefaultOnOff
}tpd_enum_st;


void SendData2ScreenTouch(tpd_enum_st data,uint16_t adr, uint16_t sceneId, uint8_t iconId,
		uint8_t statusWeather, uint16_t temp, uint16_t hum,uint16_t pm25, uint16_t years,
		uint8_t month, uint8_t date, uint8_t day, uint8_t hours, uint8_t minute,
		uint8_t second) ;
void RspScreenTouchStatus(TS_GWIF_IncomingData * data);

void RspScreenTouchAddScene(TS_GWIF_IncomingData *data);

void RspScreenTouchEditScene(TS_GWIF_IncomingData *data);

void RspScreenTouchDelScene(TS_GWIF_IncomingData *data);

void RspScreenTouchSetWeatherOut(TS_GWIF_IncomingData *data);

void RspScreenTouchWeatherIndoor(TS_GWIF_IncomingData *data);

void RspScreenTouchTime(TS_GWIF_IncomingData *data);

void RspScreenTouchDate(TS_GWIF_IncomingData *data);

void RspScreenTouchStatusOnOffGroup(TS_GWIF_IncomingData *data);

void RequestTime(TS_GWIF_IncomingData *data);

void RequestTempHum(TS_GWIF_IncomingData *data);

void RspScreenTouchDelAllScene(TS_GWIF_IncomingData * data);

void RspScreenTouchDefaultOnOff(TS_GWIF_IncomingData * data);

#endif

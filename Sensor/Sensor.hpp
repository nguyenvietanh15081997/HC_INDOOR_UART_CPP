/*
 * SensorLight.h manage tasks-related sensor( light, PIR)
 *
 */
#ifndef SENSOR_SENSOR_HPP_
#define SENSOR_SENSOR_HPP_

#include "../Include/Include.hpp"

/*Frame of rsp data sensor*/
typedef struct lightsensorRsp
{
	uint16_t  	header;
	uint16_t  	lux;
	uint8_t  	future[4];
}lightsensorRsp_t;

typedef struct pirsensorRsp
{
	uint16_t 	header;
	uint16_t 	pir;
	uint16_t  	sceneId;
}pirsensorRsp_t;

typedef struct smokeRsp{
	uint16_t 	header;
	uint8_t 	smoke;
	uint8_t		battery;
}smokeRsp_t;

typedef struct pmsensorRsp_Temp_Hum
{
	uint16_t 	header;
	uint16_t 	hum;
	uint16_t 	temp;
}pmsensorRsp_Temp_Hum_t;

typedef struct pmsensorRsp_PM
{
	uint16_t 	header;
	uint16_t 	PM_2_5;
	uint16_t 	PM_10;
	uint16_t 	PM_1_0;
}pmsensorRsp_PM_t;

typedef struct doorRspScene{
	uint16_t 	header;
	uint16_t  	sceneId;
	uint8_t		status;
}doorRspScene_t;

typedef struct doorSensorStatus{
	uint16_t 	header;
	uint16_t 	sceneId;
	uint8_t 	status;
	uint8_t 	hangOn;
}doorSensorStatus_t;

typedef struct tempHumSensorStatus{
	uint16_t header;
	uint16_t temp;
	uint16_t hum;
}tempHumSensorStatus_t;

typedef struct{
	union{
		uint32_t data;
		struct{
			uint32_t store				:8;//8 bit not use
			uint32_t Lux_hi				:10;//10 bit lux hi
			uint32_t Lux_low			:10;//10 bit lux low
			uint32_t Light_Conditon		:3; // 7 bit low
			uint32_t Pir_Conditon		:1; // 1 bit hight
		};
	};
}RD_Sensor_data_tdef;

typedef struct powerStatus{
	uint16_t header;
	uint16_t power;
}powerStatus_t;

/*Define friend_poll*/
#define SENSOR_DESCRIP_GET     0x3082
#define SENSOR_DESCRIP_STATUS  0x51

extern uint16_t  value_Lux;

/*
 * Calculate lux of lightsensor
 *
 * @param rsp_lux two byte lux
 * @return lux value calculated
 */
unsigned int CalculateLux(unsigned int rsp_lux);

void RspDoorSensorAddScene(TS_GWIF_IncomingData *data);

void RspDoorSensorDelScene(TS_GWIF_IncomingData *data);

void RspDoorStatus(TS_GWIF_IncomingData *data);

void RspPmSensorTempHum(TS_GWIF_IncomingData *data);

void RspPmSensor(TS_GWIF_IncomingData *data);

void RspTempHumSensor(TS_GWIF_IncomingData *data);

void RspPirSenSor(TS_GWIF_IncomingData *data);

void RspSmoke(TS_GWIF_IncomingData *data);

void RspPir_LightAddScene(TS_GWIF_IncomingData *data);

void RspPir_LightDelScene(TS_GWIF_IncomingData *data);

void RspPirTimeAction(TS_GWIF_IncomingData *data);

void RspPowerStatusSensor(TS_GWIF_IncomingData *data);

#endif

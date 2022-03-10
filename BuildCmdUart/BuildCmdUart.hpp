#ifndef BUILDCMDUART_HPP_
#define BUILDCMDUART_HPP_

#include "../Include/Include.hpp"
#include "../ProcessUart/OpCode.h"

/*header set, get, ask typed device*/
#define HEADER_TYPE_SET  	(0x0001)
#define HEADER_TYPE_SAVEGW  (0x0002)
#define HEADER_TYPE_ASK  	(0x0003)

/*header control scene and mode RGB*/
#define HEADER_SCENE_SET 				(0x0100)
#define HEADER_SCENE_DEL       			(0x0200)
#define HEADER_SCENE_CALL_MODE 			(0x0300)
#define HEADER_SCENE_CALL_SCENE_RGB  	(0x0000)

/*
 * TODO:AFTER 28/4
 */
/*header store scene for remote AD, DC*/
#define HEADER_SCENE_REMOTE_AC_SET 				(0x0103)
#define HEADER_SCENE_REMOTE_AC_DEL 				(0x0203)
#define HEADER_SCENE_REMOTE_DC_SET 				(0x0102)
#define HEADER_SCENE_REMOTE_DC_DEL 				(0x0202)

/*header store scene for lightsensor*/
#define HEADER_SCENE_LIGHT_SENSOR_SET 			(0x0104)
#define HEADER_SCENE_LIGHT_SENSOR_DEL 			(0x0204)
#define HEADER_SCENE_LIGHT_SENSOR_SET_BALANCE 	(0x0304)

/*header store scene for PirSensor*/
#define HEADER_SCENE_PIR_SENSOR_SET 			(0x0105)
#define HEADER_SCENE_PIR_SENSOR_DEL 			(0x0205)
#define HEADER_SCENE_PIR_SENSOR_SET_SPECIAL 	(0x0305)

/*header store scene for LightPir sensor*/
#define HEADER_SCENE_LIGHT_PIR_SET				(0x0145)
#define HEADER_SCENE_LIGHT_PIR_DEL				(0x0245)
#define HEADER_SCENE_PIR_SENSOR_TIMEACTION		(0x0345)

/*header store scene for door sensor*/
#define HEADER_SCENE_DOOR_SENSOR_SET			(0x0109)
#define HEADER_SCENE_DOOR_SENSOR_DEL 			(0x0209)


/*header store scene for Sockets1*/
#define HEADER_SCENE_SOCKET1_SET				(0x010F)
#define HEADER_SCENE_SOCKET1_DEL                (0x020F)

/*header store scene for Sockets4*/
#define HEADER_SCENE_SOCKET4_SET				(0x0110)
#define HEADER_SCENE_SOCKET4_DEL                (0x0210)

/*header send temp/hum time for screen touch*/
#define HEADER_SCENE_SCREENT_SET				(0x010A)
#define HEADER_SCENE_SCREENT_DEL				(0x020A)
#define HEADER_TEMPHUM_SCREENTOUCH    			(0x030A)
#define HEADER_TIME_SCREENTOUCH					(0x040A)

/*header ask temp, hum, pm in pm sensor*/
#define HEADER_ASK_PM							(0xE405)

/*header resetall*/
#define HEADER_RESET_ALL						(0xFFFF)

/*header of curtains*/
#define HEADER_CONTROL_CURTAIN                	(0x0011)
#define HEADER_SET_SCENE_CURTAIN				(0x0111)
#define HEADER_DEL_SCENE_CURTAIN				(0x0211)
#define HEADER_ASK_STATUS_CURTAIN      			(0x0311)

/*header of IR*/
#define HEADER_CONTROL_IR						(0xFF00)
#define HEADER_LEARNING_IR   					(0xFF01)

/*header dim percent*/
#define HEADER_DIM_PERCENT						(0xEEEE)

#define NULL8    0x00
#define NULL16   0x0000

#define LIGHTOPCODE_UPDATE      0x0182
#define LIGHTOPCODE_SELECT      0x6d82
#define LIGHTOPCODE_ONOFF       0x0282
#define LIGHTOPCODE_DIM         0x4d82
#define LIGHTOPCODE_SENCE_SET   0x4382
#define LIGHTOPCODE_SENCE_GET   0x4382
#define LIGHTOPCODE_SENCE_DEL   0x9e82

//status
#define LIGHTOPCODE_STATUS_ON   0x01
#define LIGHTOPCODE_STATUS_OFF  0x00
//
#define LIGHTNESS_GET		        	0x4B82
#define LIGHTNESS_SET		        	0x4C82
#define LIGHTNESS_SET_NOACK				0x4D82
#define LIGHTNESS_STATUS		    	0x4E82
#define LIGHTNESS_LINEAR_GET		    0x4F82
#define LIGHTNESS_LINEAR_SET		    0x5082
#define LIGHTNESS_LINEAR_SET_NOACK		0x5182
#define LIGHTNESS_LINEAR_STATUS		    0x5282
#define LIGHTNESS_LAST_GET		    	0x5382
#define LIGHTNESS_LAST_STATUS		    0x5482
#define LIGHTNESS_DEFULT_GET		    0x5582
#define LIGHTNESS_DEFULT_STATUS		    0x5682
#define LIGHTNESS_RANGE_GET		    	0x5782
#define LIGHTNESS_RANGE_STATUS		    0x5882
#define LIGHTNESS_DEFULT_SET		    0x5982
#define LIGHTNESS_DEFULT_SET_NOACK		0x5A82
#define LIGHTNESS_RANGE_SET		    	0x5B82
#define LIGHTNESS_RANGE_SET_NOACK		0x5C82
#define LIGHT_CTL_GET		        	0x5D82
#define LIGHT_CTL_SET		        	0x5E82
#define LIGHT_CTL_SET_NOACK				0x5F82
#define LIGHT_CTL_STATUS		    	0x6082
#define LIGHT_CTL_TEMP_GET		        0x6182
#define LIGHT_CTL_TEMP_RANGE_GET		0x6282
#define LIGHT_CTL_TEMP_RANGE_STATUS		0x6382
#define LIGHT_CTL_TEMP_SET				0x6482
#define LIGHT_CTL_TEMP_SET_NOACK		0x6582
#define LIGHT_CTL_TEMP_STATUS			0x6682
#define LIGHT_CTL_DEFULT_GET			0x6782
#define LIGHT_CTL_DEFULT_STATUS		    0x6882
#define LIGHT_CTL_DEFULT_SET			0x6982
#define LIGHT_CTL_DEFULT_SET_NOACK		0x6A82
#define LIGHT_CTL_TEMP_RANGE_SET		0x6B82
#define LIGHT_CTL_TEMP_RANGE_SET_NOACK	0x6C82
#define LIGHT_HSL_STATUS                0x7882
#define LIGHT_HSL_SET_NOACK				0x7782

#define SENSOR_CONDITION_PUSH_BIGGER    6
#define SENSOR_CONDITION_PUSH_LESS		4
#define SENSOR_CONDITION_PUSH_EQUAL     1
#define SENSOR_CONDITION_PUSH_BOTH		7


/* enums to is parameter for void FunctionPer()  */
typedef enum{
	null 						,
	ResetNode_typedef 			,
	ResetAll_typedef			,
	Lightness_Get_typedef 		,
	AddGroup_typedef 			,
	DelGroup_typedef 			,
	ControlOnOff_typedef 		,
	SetTimePoll_typedef 		,
	UpdateLight_typedef 		,
	CCT_Get_typedef 			,
	CCT_Set_typedef 			,
	Lightness_Set_typedef 		,
	AddSence_typedef 			,
	DelSence_typedef 			,
	CallSence_typedef 			,
	HSL_Get_typedef 			,
	HSL_Set_typedef 			,
	ControlOnoff_NoAck_typedef	,
	CCT_Set_NoAck_typedef		,
	Lightness_Set_NoAck_typedef	,
	HSL_Set_NoAck_typedef		,

	SceneForRemote_DC_vendor_typedef 			,
	SceneForRemote_AC_vendor_typedef 			,
	DelSceneForRemote_DC_vendor_typedef 		,
	DelSceneForRemote_AC_vendor_typedef 		,

	SceneForSensor_LightPir_vendor_typedef 		,
	DelSceneForSensor_LightPir_vendor_typedef   ,
	SetTimeAction_vendor_typedef				,

	SceneForSensor_Light_vendor_typedef 		,
	Balance_Lightness_vendor_typedef			,
	DelBalance_Lightness_vendor_typedef         ,
	DelSceneForSensor_Light_vendor_typedef    	,

	SceneForSensor_Pir_vendor_typedef 			,
	DelSceneForSensor_Pir_vendor_typedef    	,

	SceneForDoorSensor_vendor_typedef			,
	DelSceneForDoorSensor_vendor_typedef		,

	ControlSwitch4_vendor_typedef				,
	SetSceneForSwitch4_vendor_typedef			,
	DelSceneForSwitch4_vendor_typedef			,

	ControlSocket1_vendor_typedef				,
	SetSceneForSocket1_vendor_typedef			,
	DelSceneForSocket1_vendor_typedef			,

	SceneForScreenT_vendor_typedef				,
	DelSceneForScreenT_vendor_typedef			,
	SendTempHumForScreenT_vendor_typedef		,
	SendTimeForScreenT_vendor_typedef			,

	ControlCurtain_vendor_typedef				,
	SetSceneCurtain_vendor_typedef				,
	DelSceneCurtain_vendor_typedef				,
	AskSceneCurtain_vendor_typedef				,

	SceneForRGB_vendor_typedef 	  		,
	CallSceneRgb_vendor_typedef   		,
	CallModeRgb_vendor_typedef    		,
	DelSceneRgb_vendor_typedef    		,

	SaveGateway_vendor_typedef    ,
	AskTypeDevice_vendor_typedef  ,
	SetTypeDevice_vendor_typedef  ,
	AskPm_vendor_typedef,

	DimPercent_vendor_typedef,

	IRControl_vendor_typedef,
	IRLearning_vendor_typedef,
	IrSendData_vendor_typedef
} functionTypeDef;

typedef enum{
	update_OnOff,
	update_DIM_CCT,
	update_HSL,
	update_Null
}typeUpdate;
/*
 * Update light
 */
void CmdUpdateLight(typeUpdate type, uint16_t cmd, uint16_t adr, uint16_t cmdLength);

/*
 * Main function: build frame data control node, and transmit uart
 * Enter the appropriate parameters to specify the control command
 * When built frame data control, function transmit uart
 *
 *@param cmd two byte first to define control commands
 *@param Func typedefenum to call above functions
 *@param unicastAdr address node in ble mesh
 *@param adrGroup address group
 *@param parStatusOnOff status on/off
 *@param parLightness value lightness
 *@param parCCT value cct
 *@param parSenceId id scene
 *@param parTimePoll timepoll to sensor
 *@param parH value Hue
 *@param parS value Saturation
 *@param parL value Lightness
 *@param cmdLenght the length of the commands
 *@return null
 */
void FunctionPer(uint16_t cmd,\
				functionTypeDef Func,\
				uint16_t unicastAdr,\
				uint16_t adrGroup,\
				uint8_t  parStatusOnOff,\
				uint16_t parLightness,\
				uint16_t parCCT,\
				uint16_t parSenceId,\
				uint16_t parTimePoll,\
				uint16_t parL,\
				uint16_t parH,\
				uint16_t parS,\
				uint16_t transition_par_t,\
				uint8_t cmdLength);


/*Control opcode vendor*/
void Function_Vendor(uint16_t cmd,\
		functionTypeDef Func_vendor,\
		uint16_t 	adr,\
		uint16_t 	header_destination,\
		uint8_t 	buttonID,\
		uint8_t 	modeID,\
		uint8_t 	status_door,\
		uint16_t 	condition_lightness,\
		uint16_t 	low_lux_switch1_2_socket1_2,\
		uint16_t 	hight_lux_switch3_4_socket3_4,\
		uint16_t 	temp,\
		uint16_t 	hum,\
		uint16_t 	sceneID,\
		uint16_t 	appID,\
		uint8_t 	srgbID,\
		uint8_t 	type_hours,\
		uint8_t 	attrubute_minute,\
		uint8_t 	application_second,\
		uint16_t 	transition_par_t,\
		uint16_t 	cmdLength
		);
#endif 

/*
 * GatewayInterface.c
 */

#include "GateInterface.hpp"
#include "../Remote/Remote.hpp"
#include "../Sensor/Sensor.hpp"
#include "../Light/Light.hpp"
#include "../ScreenTouch/ScreenTouch.hpp"
#include "../SwitchOnOff/SwitchOnOff.hpp"
#include "../BuildCmdUart/BuildCmdUart.hpp"
#include "../logging/slog.h"
#include "Provision.hpp"
#include <sys/time.h>

#include <fcntl.h> // Contains file controls like O_RDWR
#include <errno.h> // Error integer and strerror() function
#include <termios.h> // Contains POSIX terminal control definitions
#include <unistd.h>



static ringbuffer_t 		vrts_ringbuffer_Data;
static TS_GWIF_IncomingData	*vrts_GWIF_IncomeMessage;
static unsigned char		vrsc_GWIF_TempBuffer[TEMPBUFF_LENGTH] = {0};
static uint16_t				vrui_GWIF_LengthMeassge;
static bool					vrb_GWIF_CheckNow = false;
static bool					vrb_GWIF_RestartMessage = true;
static bool 				message_Update = false;

int serial_port;

static char* ToChar(cmdcontrol_t data) {
	char * temp = (char *)(&data.HCI_CMD_GATEWAY[0]);
	return temp;
}
static void Uart_Init(){
	serial_port = open("/dev/ttyS1", O_RDWR);
	struct termios tty;
	// Read in existing settings, and handle any error
	if(tcgetattr(serial_port, &tty) != 0) {
	  printf("Error %i from tcgetattr: %s\n", errno, strerror(errno));
	  //return 1;
	}
	tty.c_cflag &= ~PARENB; // Clear parity bit, disabling parity (most common)
	tty.c_cflag &= ~CSTOPB; // Clear stop field, only one stop bit used in communication (most common)
	tty.c_cflag &= ~CSIZE; // Clear all bits that set the data size
	tty.c_cflag |= CS8; // 8 bits per byte (most common)
	tty.c_cflag &= ~CRTSCTS; // Disable RTS/CTS hardware flow control (most common)
	tty.c_cflag |= CREAD | CLOCAL; // Turn on READ & ignore ctrl lines (CLOCAL = 1)

	tty.c_lflag &= ~ICANON;
	tty.c_lflag &= ~ECHO; // Disable echo
	tty.c_lflag &= ~ECHOE; // Disable erasure
	tty.c_lflag &= ~ECHONL; // Disable new-line echo
	tty.c_lflag &= ~ISIG; // Disable interpretation of INTR, QUIT and SUSP
	tty.c_iflag &= ~(IXON | IXOFF | IXANY); // Turn off s/w flow ctrl
	tty.c_iflag &= ~(IGNBRK|BRKINT|PARMRK|ISTRIP|INLCR|IGNCR|ICRNL); // Disable any special handling of received bytes

	tty.c_oflag &= ~OPOST; // Prevent special interpretation of output bytes (e.g. newline chars)
	tty.c_oflag &= ~ONLCR; // Prevent conversion of newline to carriage return/line feed
	// tty.c_oflag &= ~OXTABS; // Prevent conversion of tabs to spaces (NOT PRESENT ON LINUX)
	// tty.c_oflag &= ~ONOEOT; // Prevent removal of C-d chars (0x004) in output (NOT PRESENT ON LINUX)

	tty.c_cc[VTIME] = 0;    // Wait for up to 1s (10 deciseconds), returning as soon as any data is received.
	tty.c_cc[VMIN] = 0;

	// Set in/out baud rate to be 9600
	cfsetispeed(&tty, B115200);
	cfsetospeed(&tty, B115200);

	// Save tty settings, also checking for error
	if (tcsetattr(serial_port, TCSANOW, &tty) != 0) {
	  printf("Error %i from tcsetattr: %s\n", errno, strerror(errno));
	}
}

static void GWIF_Init (void){
	ring_init(&vrts_ringbuffer_Data, RINGBUFFER_LEN, sizeof(uint8_t));
	Uart_Init();
	vrts_GWIF_IncomeMessage = (TS_GWIF_IncomingData *)vrsc_GWIF_TempBuffer;
}

static uint64_t getMillisOfDay(){
	uint64_t millis;
	struct timeval timeCurrent;

	gettimeofday(&timeCurrent,NULL);
	millis = (timeCurrent.tv_sec * 1000)+ (timeCurrent.tv_usec /1000);

	return millis;
}

#if 0
static uint16_t getSecondDay(){
	uint64_t second;
	struct timeval timeCurrent;

	gettimeofday(&timeCurrent,NULL);
	second = (timeCurrent.tv_sec);
	return second;
}
#endif

static void LogDataUart(bool hasRsp, uint8_t length, void *data){
	unsigned char buffer[length+1];
	memcpy(buffer,data,length);
	unsigned char temp[10] = {0};
	unsigned char dataLog[200] = {0};
	for(int i = 0; i < length; i++){
		sprintf((char *)temp, "%02x ",buffer[i]);
		strcat((char *)dataLog,(char *)temp);
	}
	if(hasRsp){
		slog_print(SLOG_TRACE, 1, "<uart>rsp:%s",dataLog);
	}else {
		slog_print(SLOG_TRACE, 1, "<uart>cmd:%s",dataLog);
	}
}

static uint64_t t1, t2, t3, t4;
static uint64_t waitCmd, waitUpdate;
static void GWIF_WriteMessage(void) {
	pthread_mutex_trylock(&vrpth_SendUart);
	if (bufferDataUart.size()) {
		uartSendDev_t data = bufferDataUart.front();
		t1 = getMillisOfDay();

		if ((t1 - t2) >= waitCmd) {
			waitCmd = data.timeWait;
			write(serial_port, ToChar(data.dataUart), data.length);
			tcdrain(serial_port);
			bufferDataUart.pop_front();
			bufferDataUart.shrink_to_fit();
			t2 = t1;
			t4 = t1;

			LogDataUart(0, data.length, (void*) &data.dataUart.HCI_CMD_GATEWAY[0]);

			uint16_t opcodeCmd = data.dataUart.opCode[0] | (data.dataUart.opCode[1] << 8);
			switch (opcodeCmd){
			case SCENE_STORE:
				gvrb_AddSceneLight = true;
				break;
			case SCENE_DEL:
				gvrb_AddSceneLight = false;
				gSceneIdDel = data.dataUart.para[0] | (data.dataUart.para[1] << 8);
				break;
			case CFG_MODEL_SUB_ADD:
				gvrb_AddGroupLight = true;
				break;
			case CFG_MODEL_SUB_DEL:
				gvrb_AddGroupLight = false;
				break;
			}
		}
	}
	else if (bufferUartUpdate.size() && !gvrb_Provision){
		uartSendDev_t dataUpdate = bufferUartUpdate.front();
		t3 = getMillisOfDay();
		if((t3 - t4) >= waitUpdate) {
			waitUpdate = dataUpdate.timeWait;
			waitCmd = dataUpdate.timeWait;
			write(serial_port, ToChar(dataUpdate.dataUart), dataUpdate.length);
			tcdrain(serial_port);
			bufferUartUpdate.pop_front();
			bufferUartUpdate.shrink_to_fit();
			t4 = t3;
			t2 = t3;
			LogDataUart(0, dataUpdate.length, (void*) &dataUpdate.dataUart.HCI_CMD_GATEWAY[0]);
		}
	}
	pthread_mutex_unlock(&vrpth_SendUart);
}

static uint8_t read_buf[256];
static int num_bytes;
static int num_count2Push;
static uint16_t countReadUart = 0;

static void GWIF_Read2Buffer (void){
	if(num_bytes == 0){
		num_bytes = read(serial_port, &read_buf,128);
		num_count2Push = 0;
		while((vrts_ringbuffer_Data.count < RINGBUFFER_LEN) && (num_bytes > 0)){
			ring_push_head((ringbuffer_t *)&vrts_ringbuffer_Data,(void *)(&read_buf[num_count2Push]));
			num_bytes--;
			num_count2Push++;
		}
	}
	else{
		while((vrts_ringbuffer_Data.count < RINGBUFFER_LEN) && (num_bytes > 0)){
			ring_push_head((ringbuffer_t *)&vrts_ringbuffer_Data,(void *)(&read_buf[num_count2Push]));
			num_bytes--;
			num_count2Push++;
		}
	}

}

static void GWIF_CheckData()
{
	unsigned char vrui_Count;
	if(vrts_ringbuffer_Data.count >= 1){

		if (vrb_GWIF_RestartMessage){
			if(vrts_ringbuffer_Data.count >= 4){
				vrb_GWIF_RestartMessage = false;
				ring_pop_tail(&vrts_ringbuffer_Data,(void*) &vrsc_GWIF_TempBuffer[0]);
				ring_pop_tail(&vrts_ringbuffer_Data,(void*) &vrsc_GWIF_TempBuffer[1]);
				ring_pop_tail(&vrts_ringbuffer_Data,(void*) &vrsc_GWIF_TempBuffer[2]);
				vrui_GWIF_LengthMeassge = vrts_GWIF_IncomeMessage->Length[0] | (vrts_GWIF_IncomeMessage->Length[1] << 8);
				message_Update = true;
			}
		}
		if (message_Update) {
			message_Update = false;
			if(vrui_GWIF_LengthMeassge >= 3){
				if ((vrts_GWIF_IncomeMessage->Opcode == TSCRIPT_MESH_RX)
						|| (vrts_GWIF_IncomeMessage->Opcode == TSCRIPT_MESH_RX_NW)
						|| (vrts_GWIF_IncomeMessage->Opcode == TSCRIPT_GATEWAY_DIR_RSP)
						|| (vrts_GWIF_IncomeMessage->Opcode == HCI_GATEWAY_CMD_SAR_MSG)
						|| (vrts_GWIF_IncomeMessage->Opcode == TSCRIPT_CMD_VC_DEBUG))
				{
					if (vrts_ringbuffer_Data.count >= (vrui_GWIF_LengthMeassge -1)) {
						for (vrui_Count = 0; vrui_Count < vrui_GWIF_LengthMeassge - 1; vrui_Count++) {
							ring_pop_tail(&vrts_ringbuffer_Data, (void*) &vrsc_GWIF_TempBuffer[MESSAGE_HEADLENGTH + vrui_Count]);
						}
						vrb_GWIF_CheckNow = true;
						vrb_GWIF_RestartMessage = true;
						message_Update = false;
					}
					else {
						message_Update = true;
					}
				}
				else {
					vrsc_GWIF_TempBuffer[0] = vrsc_GWIF_TempBuffer[1];
					vrsc_GWIF_TempBuffer[1] = vrsc_GWIF_TempBuffer[2];
					ring_pop_tail(&vrts_ringbuffer_Data,(void*) &vrsc_GWIF_TempBuffer[2]);
					vrui_GWIF_LengthMeassge = vrts_GWIF_IncomeMessage->Length[0] | (vrts_GWIF_IncomeMessage->Length[1] << 8);
					message_Update = true;
				}
			}
			else {
				vrsc_GWIF_TempBuffer[0] = vrsc_GWIF_TempBuffer[1];
				vrsc_GWIF_TempBuffer[1] = vrsc_GWIF_TempBuffer[2];
				ring_pop_tail(&vrts_ringbuffer_Data,(void*) &vrsc_GWIF_TempBuffer[2]);
				vrui_GWIF_LengthMeassge = vrts_GWIF_IncomeMessage->Length[0] | (vrts_GWIF_IncomeMessage->Length[1] << 8);
				message_Update = true;
			}
		}
	}
}

typedef void (*cb_rsp_function_t)(TS_GWIF_IncomingData * data);

typedef struct processRsp0x52{
	uint16_t 			header;
	cb_rsp_function_t 	rspFuncProcess0x52;
} proccessRsp0x52_t;

#define MAX_FUNCTION0x52_RSP			17
proccessRsp0x52_t listRspFunction0x52[MAX_FUNCTION0x52_RSP] = {
		{REMOTE_MODULE_AC_TYPE,				RspRemoteStatus},
		{REMOTE_MODULE_DC_TYPE,				RspRemoteStatus},
		{SCREEN_TOUCH_MODULE_TYPE,			RspScreenTouchStatus},
		{POWER_TYPE,						RspPowerStatusSensor},
		{ST_HEADER_ONOFF_GROUP,				RspScreenTouchStatusOnOffGroup},
		{PIR_SENSOR_MODULE_TYPE,			RspPirSenSor},
		{PM_SENSOR_MODULE_TEMP_HUM_TYPE,	RspPmSensorTempHum},
		{PM_SENSOR_MODULE_PM_TYPE,			RspPmSensor},
		{TEMP_HUM_MODULE_TYPE,				RspTempHumSensor},
		{DOOR_SENSOR_MODULE_TYPE,			RspDoorStatus},
		{DOOR_SENSOR_HANGON_MODULE_TYPE,	RspDoorHangOn},
		{SMOKE_SENSOR_MODULE_TYPE,			RspSmoke},
		{LIGHT_SENSOR_MODULE_TYPE,			RspLightSensor},
		{SWITCH_1_CONTROL,					Rsp_Switch_Status},
		{SWITCH_2_CONTROL,					Rsp_Switch_Status},
		{SWITCH_3_CONTROL,					Rsp_Switch_Status},
		{SWITCH_4_CONTROL,					Rsp_Switch_Status}
};

typedef struct processRsp{
	uint16_t 			opcode;
	cb_rsp_function_t 	rspFuncProcess;
} proccessRsp_t;

#define MAX_FUNCTION_RSP			11
proccessRsp_t listRspFunction[MAX_FUNCTION_RSP] = {
		{CFG_DEFAULT_TTL_STATUS,			RspTTL},
		{G_ONOFF_STATUS,					RspOnoff},
		{LIGHT_CTL_TEMP_STATUS,				RspCCT},
		{LIGHTNESS_STATUS,					RspDIM},
		{LIGHT_HSL_STATUS,					RspHSL},
		{CFG_MODEL_SUB_STATUS,			   	RspAddDelGroup},
		{SCENE_REG_STATUS,					RspAddDelSceneLight},
		{SCENE_STATUS,						RspCallScene},
		{G_BATTERY_STATUS,					RspPowerRemoteStatus},
		{NODE_RESET_STATUS,					RspResetNode},
		{LIGHTNESS_LINEAR_STATUS,			RspCallModeRgb}
};

typedef struct processRspVendor{
	uint16_t 			headerVendor;
	cb_rsp_function_t 	rspFuncVendorProcess;
} proccessRspVendor_t;

#define MAX_FUNCTIONVENDOR_RSP			32
proccessRspVendor_t listRspFunctionVendor[MAX_FUNCTIONVENDOR_RSP] = {
		{HEADER_TYPE_ASK,						RspTypeDevice},
		{HEADER_TYPE_SET,						RspTypeDevice},
		{HEADER_TYPE_SAVEGW,					RspSaveGw},

//		{HEADER_SCENE_CALL_MODE,				RspCallModeRgb},
//		{HEADER_SCENE_DEL,			   			RspDelSceneRgb},
//		{HEADER_SCENE_SET,						RspAddSceneRgb},
//		{HEADER_SCENE_CALL_SCENE_RGB,			RspCallSceneRgb},
		{HEADER_SCENE_REMOTE_DC_SET,			RspAddSceneRemote},
		{HEADER_SCENE_REMOTE_DC_DEL,			RspRemoteDelScene},
		{HEADER_SCENE_REMOTE_AC_SET,			RspAddSceneRemote},
		{HEADER_SCENE_REMOTE_AC_DEL,			RspRemoteDelScene},
		{HEADER_SCENE_LIGHT_PIR_SET,			RspPir_LightAddScene},
		{HEADER_SCENE_LIGHT_PIR_DEL,			RspPir_LightDelScene},
		{HEADER_SCENE_PIR_SENSOR_TIMEACTION,	RspPirTimeAction},
//		{HEADER_SCENE_DOOR_SENSOR_SET,			RspDoorSensorAddScene},
//		{HEADER_SCENE_DOOR_SENSOR_DEL,			RspDoorSensorDelScene},
		{SWITCH_1_CONTROL,						Rsp_Switch_Control},
		{SWITCH_1_SCENE_SET,					Rsp_Switch_Scene_Set},
		{SWITCH_1_SCENE_DEL,					Rsp_Switch_Scene_Del},
		{SWITCH_2_CONTROL,						Rsp_Switch_Control},
		{SWITCH_2_SCENE_SET,					Rsp_Switch_Scene_Set},
		{SWITCH_2_SCENE_DEL,					Rsp_Switch_Scene_Del},
		{SWITCH_3_CONTROL,						Rsp_Switch_Control},
		{SWITCH_3_SCENE_SET,					Rsp_Switch_Scene_Set},
		{SWITCH_3_SCENE_DEL,					Rsp_Switch_Scene_Del},
		{SWITCH_4_CONTROL,						Rsp_Switch_Control},
		{SWITCH_4_SCENE_SET,					Rsp_Switch_Scene_Set},
		{SWITCH_4_SCENE_DEL,					Rsp_Switch_Scene_Del},
		{ST_HEADER_ADD_SCENE,					RspScreenTouchAddScene},
		{ST_HEADER_DEL_SCENE,					RspScreenTouchDelScene},
		{ST_HEADER_DELALL_SCENE,				RspScreenTouchDelAllScene},
		{ST_HEADER_DEFAULT_ONOFF,				RspScreenTouchDefaultOnOff},
		{ST_HEADER_GET_WEATHER_OUTDOOR,			RspScreenTouchSetWeatherOut},
		{ST_HEADER_GET_WEATHER_INDOOR,			RspScreenTouchWeatherIndoor},
		{ST_HEADER_GET_TIME,					RspScreenTouchTime},
		{ST_HEADER_GET_DATE,					RspScreenTouchDate},
		{ST_HEADER_REQUEST_TIME,				RequestTime},
		{ST_HEADER_REQUEST_TEMP_HUM,			RequestTempHum}
//		{HEADER_CONTROL_CURTAIN,				RspControlCurtain},
//		{HEADER_SET_SCENE_CURTAIN,				RspCurtainAddScene},
//		{HEADER_DEL_SCENE_CURTAIN,				RspCurtainDelScene},
//		{HEADER_ASK_STATUS_CURTAIN,				RspCurtainStatus},
};

static int GWIF_ProcessData (void)
{
	if (vrb_GWIF_CheckNow) {
		vrb_GWIF_CheckNow = false;
		if(vrts_GWIF_IncomeMessage->Message[0] != 0x88){
			LogDataUart(1, (vrui_GWIF_LengthMeassge + 2), (void*) &vrts_GWIF_IncomeMessage->Length[0]);
		}
		if (gvrb_Provision) {
			if ((vrts_GWIF_IncomeMessage->Message[0] == HCI_GATEWAY_CMD_UPDATE_MAC) && \
					(stateProvision == statePro_findDev))
			{
				for (int i = 0; i < 6; i++) {
					OUTMESSAGE_MACSelect[i + 3] = vrts_GWIF_IncomeMessage->Message[i + 1];
				}
				sprintf((char *)PRO_mac, "%02x:%02x:%02x:%02x:%02x:%02x",
						vrts_GWIF_IncomeMessage->Message[1],
						vrts_GWIF_IncomeMessage->Message[2],
						vrts_GWIF_IncomeMessage->Message[3],
						vrts_GWIF_IncomeMessage->Message[4],
						vrts_GWIF_IncomeMessage->Message[5],
						vrts_GWIF_IncomeMessage->Message[6]);
				sprintf((char*) PRO_uuid,
						"%02x%02x%02x%02x-%02x%02x-%02x%02x-%02x%02x-%02x%02x%02x%02x%02x%02x",
						vrts_GWIF_IncomeMessage->Message[10],
						vrts_GWIF_IncomeMessage->Message[11],
						vrts_GWIF_IncomeMessage->Message[12],
						vrts_GWIF_IncomeMessage->Message[13],
						vrts_GWIF_IncomeMessage->Message[14],
						vrts_GWIF_IncomeMessage->Message[15],
						vrts_GWIF_IncomeMessage->Message[16],
						vrts_GWIF_IncomeMessage->Message[17],
						vrts_GWIF_IncomeMessage->Message[18],
						vrts_GWIF_IncomeMessage->Message[19],
						vrts_GWIF_IncomeMessage->Message[20],
						vrts_GWIF_IncomeMessage->Message[21],
						vrts_GWIF_IncomeMessage->Message[22],
						vrts_GWIF_IncomeMessage->Message[23],
						vrts_GWIF_IncomeMessage->Message[24],
						vrts_GWIF_IncomeMessage->Message[25]);
				slog_print(SLOG_INFO,1,"%s",(char *)PRO_uuid);
				stateProvision = statePro_selectMac;

			} else if ((vrts_GWIF_IncomeMessage->Message[0] == HCI_GATEWAY_CMD_PRO_STS_RSP)
					&& (vrts_GWIF_IncomeMessage->Message[21] != 0x11)
					&& (vrts_GWIF_IncomeMessage->Message[22] != 0x22)
					&& (vrts_GWIF_IncomeMessage->Message[23] != 0x33)
					&& (vrts_GWIF_IncomeMessage->Message[24] != 0x44)) {
				stateProvision = statePro_setPro;
//			}
//			else if ((vrts_GWIF_IncomeMessage->Message[0] == HCI_GATEWAY_CMD_SETPRO_SUSCESS) && \
//					(stateProvision == statePro_waitSetPro)){
//				stateProvision = statePro_getPro;
			} else if ((vrts_GWIF_IncomeMessage->Message[0] == HCI_GATEWAY_CMD_PRO_STS_RSP) &&\
						(vrts_GWIF_IncomeMessage->Message[21] == 0x11) &&\
						(vrts_GWIF_IncomeMessage->Message[22] == 0x22) &&\
						(vrts_GWIF_IncomeMessage->Message[23] == 0x33) &&\
						(vrts_GWIF_IncomeMessage->Message[24] == 0x44)) {
				OUTMESSAGE_Provision[0] = HCI_CMD_GATEWAY_CTL & 0xFF;    //0xE9
				OUTMESSAGE_Provision[1] = (HCI_CMD_GATEWAY_CTL >> 8) & 0xFF; //0xFF;
				OUTMESSAGE_Provision[2] = HCI_GATEWAY_CMD_SET_NODE_PARA;
				if ((vrts_GWIF_IncomeMessage->Message[25] == 0x00) && \
						(vrts_GWIF_IncomeMessage->Message[26] == 0x00)) {
					for (int i = 0; i < 23; i++) {
						OUTMESSAGE_Provision[i + 3] = vrts_GWIF_IncomeMessage->Message[i + 2];
					}
					OUTMESSAGE_Provision[26] = 0x02;
					OUTMESSAGE_Provision[27] = 0x00;
					adr_Provision = 2;
				} else {
					for (int i = 0; i < 25; i++) {
						OUTMESSAGE_Provision[i + 3] = vrts_GWIF_IncomeMessage->Message[i + 2];
					}
					adr_Provision = OUTMESSAGE_Provision[26] | (OUTMESSAGE_Provision[27] << 8);
				}
				sprintf((char*) PRO_netKey,
						"%02x%02x%02x%02x-%02x%02x-%02x%02x-%02x%02x-%02x%02x%02x%02x%02x%02x",
						vrts_GWIF_IncomeMessage->Message[2],
						vrts_GWIF_IncomeMessage->Message[3],
						vrts_GWIF_IncomeMessage->Message[4],
						vrts_GWIF_IncomeMessage->Message[5],
						vrts_GWIF_IncomeMessage->Message[6],
						vrts_GWIF_IncomeMessage->Message[7],
						vrts_GWIF_IncomeMessage->Message[8],
						vrts_GWIF_IncomeMessage->Message[9],
						vrts_GWIF_IncomeMessage->Message[10],
						vrts_GWIF_IncomeMessage->Message[11],
						vrts_GWIF_IncomeMessage->Message[12],
						vrts_GWIF_IncomeMessage->Message[13],
						vrts_GWIF_IncomeMessage->Message[14],
						vrts_GWIF_IncomeMessage->Message[15],
						vrts_GWIF_IncomeMessage->Message[16],
						vrts_GWIF_IncomeMessage->Message[17]);
				stateProvision = statePro_provision;
			} else if (vrts_GWIF_IncomeMessage->Message[0] == HCI_GATEWAY_CMD_PROVISION_EVT
					&& vrts_GWIF_IncomeMessage->Message[1] == HCI_GATEWAY_CMD_PROVISION_SUSCESS) {
				stateProvision = statePro_bindingAll;
			} else if((vrui_GWIF_LengthMeassge == 27) && (vrts_GWIF_IncomeMessage->Message[0] == 0xb5)){
				sprintf((char*) PRO_appKey,
						"%02x%02x%02x%02x-%02x%02x-%02x%02x-%02x%02x-%02x%02x%02x%02x%02x%02x",
						vrts_GWIF_IncomeMessage->Message[10],vrts_GWIF_IncomeMessage->Message[11],
						vrts_GWIF_IncomeMessage->Message[12],vrts_GWIF_IncomeMessage->Message[13],
						vrts_GWIF_IncomeMessage->Message[14],vrts_GWIF_IncomeMessage->Message[15],
						vrts_GWIF_IncomeMessage->Message[16],vrts_GWIF_IncomeMessage->Message[17],
						vrts_GWIF_IncomeMessage->Message[18],vrts_GWIF_IncomeMessage->Message[19],
						vrts_GWIF_IncomeMessage->Message[20],vrts_GWIF_IncomeMessage->Message[21],
						vrts_GWIF_IncomeMessage->Message[22],vrts_GWIF_IncomeMessage->Message[23],
						vrts_GWIF_IncomeMessage->Message[24],vrts_GWIF_IncomeMessage->Message[25]);
			} else if(vrts_GWIF_IncomeMessage->Message[0] == HCI_GATEWAY_CMD_KEY_BIND_EVT && vrts_GWIF_IncomeMessage->Message[1] == HCI_GATEWAY_CMD_BIND_SUSCESS){
				slog_info("<provision> SUCCESS");
			} else if (vrts_GWIF_IncomeMessage->Message[0] == HCI_GATEWAY_KEY_BIND_RSP){
				stateProvision = statePro_saveGw;
			} else if (vrts_GWIF_IncomeMessage->Message[0] == HCI_GATEWAY_RSP_OP_CODE) {
				if (vrts_GWIF_IncomeMessage->Message[5] == RD_OPCODE_TYPE_RSP) {
					if (VENDOR_ID == (vrts_GWIF_IncomeMessage->Message[6] | (vrts_GWIF_IncomeMessage->Message[7] << 8))) {
						uint16_t headVendor = vrts_GWIF_IncomeMessage->Message[8] | (vrts_GWIF_IncomeMessage->Message[9] << 8);
						if (headVendor == HEADER_TYPE_SAVEGW) {
							stateProvision = statePro_setType;
							RspSaveGw(vrts_GWIF_IncomeMessage);
						}
						else if((headVendor == HEADER_TYPE_SET) || (headVendor == HEADER_TYPE_ASK)){
							stateProvision = statePro_scan;
							RspTypeDevice(vrts_GWIF_IncomeMessage);
						}
					}
				} else if (vrts_GWIF_IncomeMessage->Message[5] == RD_OPCODE_SCENE_RSP){
					if (VENDOR_ID == (vrts_GWIF_IncomeMessage->Message[6] | (vrts_GWIF_IncomeMessage->Message[7] << 8))) {
						uint16_t headVendor = vrts_GWIF_IncomeMessage->Message[8] | (vrts_GWIF_IncomeMessage->Message[9] << 8);
						if (headVendor == ST_HEADER_REQUEST_TIME){
							RequestTime(vrts_GWIF_IncomeMessage);
						}
						else if (headVendor == ST_HEADER_REQUEST_TEMP_HUM){
							RequestTempHum(vrts_GWIF_IncomeMessage);
						}
					}
				}
			}
		}
		else {
			if(vrts_GWIF_IncomeMessage->Message[0] == HCI_GATEWAY_RSP_OP_CODE){
				uint8_t opcodeVendor = vrts_GWIF_IncomeMessage->Message[5];
				uint16_t vendorId = vrts_GWIF_IncomeMessage->Message[6] |(vrts_GWIF_IncomeMessage->Message[7]<<8);
				uint16_t headerVendor = vrts_GWIF_IncomeMessage->Message[8] | (vrts_GWIF_IncomeMessage->Message[9] << 8);
				if (opcodeVendor == SENSOR_TYPE) {
					uint16_t header = vrts_GWIF_IncomeMessage->Message[6] | (vrts_GWIF_IncomeMessage->Message[7] << 8);
					for (int i = 0; i < MAX_FUNCTION0x52_RSP; i++) {
						if (header == listRspFunction0x52[i].header) {
							listRspFunction0x52[i].rspFuncProcess0x52(vrts_GWIF_IncomeMessage);
							return 0;
						}
					}
				}
				else if (opcodeVendor == RD_OPCODE_TYPE_RSP){
					if(vendorId == VENDOR_ID){
						for (int n = 0; n < MAX_FUNCTIONVENDOR_RSP; n++) {
							if(headerVendor == listRspFunctionVendor[n].headerVendor){
								listRspFunctionVendor[n].rspFuncVendorProcess(vrts_GWIF_IncomeMessage);
								return 0;
							}
						}
					}
				}
				else if (opcodeVendor == RD_OPCODE_SCENE_RSP){
					if(vendorId == VENDOR_ID){
						for (int m = MAX_FUNCTIONVENDOR_RSP - 1; m >= 0; m--) {
							if(headerVendor == listRspFunctionVendor[m].headerVendor){
								listRspFunctionVendor[m].rspFuncVendorProcess(vrts_GWIF_IncomeMessage);
								return 0;
							}
						}
					}
				}
				uint16_t opcode = vrts_GWIF_IncomeMessage->Message[5] | (vrts_GWIF_IncomeMessage->Message[6] << 8);
				for (int j = 0; j < MAX_FUNCTION_RSP; j++) {
					if (listRspFunction[j].opcode == opcode) {
						listRspFunction[j].rspFuncProcess(vrts_GWIF_IncomeMessage);
						return 0;
					}
				}
			}
		}
	}
	return 1;
}

void* GWINF_Thread(void *vargp) {
	slog_print(SLOG_INFO, 1, "Thread uart start");
	GWIF_Init();
	while (1) {
		GWIF_WriteMessage();
		GWIF_Read2Buffer();
		GWIF_CheckData();
		GWIF_ProcessData();
		usleep(1000);
	}
	return NULL;
}

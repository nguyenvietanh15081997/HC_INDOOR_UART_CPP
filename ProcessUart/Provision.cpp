/*
 *Provision.c
 */

#include "Provision.hpp"
#include <sys/time.h>
#include "../logging/slog.h"
#include "../Include/Include.hpp"
#include "../BuildCmdUart/BuildCmdUart.hpp"
#include "../Mqtt/Mqtt.hpp"

#define TIMESCAN 3
clock_t startP, endP;
clock_t vrTimeout = 900000;
static bool startCountSaveGW = false;
static bool startCountSetType = false;
static bool startCountHB = false;
static bool isSetTypeDev = false;
pthread_t tmp;
pthread_t vrts_System_Gpio;

uint64_t Timeout_CheckDataBuffer1 = 0;
bool Pro_startCount = false;
unsigned char scanNotFoundDev =0 ;
unsigned int adr_heartbeat;

uint8_t OUTMESSAGE_ScanStop[3]     = {0xE9, 0xFF, 0x01};
uint8_t OUTMESSAGE_ScanStart[3]    = {0xE9, 0xFF, 0x00};
uint8_t OUTMESSAGE_MACSelect[9]    = {0xE9, 0xFF, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
uint8_t OUTMESSAGE_GetPro[3] 	   = {0xE9, 0xFF, 0x0C};
uint8_t OUTMESSAGE_Provision[28]   = {0};
uint8_t OUTMESSAGE_BindingALl[22]  = {0xe9, 0xff, 0x0b, 0x00, 0x00, 0x00, 0x60, 0x96, 0x47, 0x71, 0x73, 0x4f, 0xbd, 0x76, 0xe3, 0xb4, 0x05, 0x19, 0xd1, 0xd9, 0x4a, 0x48};

uint8_t setpro_internal[]       =   {0xe9, 0xff, 0x09, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x11, 0x22, 0x33, 0x44, 0x01, 0x00};
uint8_t admit_pro_internal[]    =   {0xe9, 0xff, 0x0d, 0x01, 0x00, 0xff, 0xfb, 0xeb, 0xbf, 0xea, 0x06, 0x09, 0x00, 0x52, 0x90, 0x49, 0xf1, 0xf1, 0xbb, 0xe9, 0xeb};// trả về unicast tiếp theo của con đèn cần thêm vào
uint8_t reset_GW[]				=	{0xe9, 0xff, 0x02};

bool isProvision 			= false;
bool flag_selectmac     	= false;
bool flag_getpro_info   	= false;
bool flag_getpro_element	= false;
bool flag_provision     	= false;
bool flag_mac           	= true;
bool flag_check_select_mac  = false;
bool flag_done          	= true;
bool flag_setpro            = false;
bool flag_admitpro          = false;
bool flag_checkadmitpro     = true;

bool flag_set_type          = false;
bool flag_checkHB           = false;
bool flag_checkSaveGW		= false;
bool flag_checkTypeDEV 		= false;
bool hasSelectMac			= true;

bool vrb_CheckingClock = false;
struct timeval vrcl_Start,vrcl_End;
uint8_t PRO_deviceKey[37] = {0};
uint8_t PRO_netKey[37] = {0};
uint8_t PRO_appKey[37] = {0};
uint8_t PRO_uuid[37] = {0};
uint8_t PRO_mac[24] = {0};

void InitFalg(){
//	ControlMessage(3,OUTMESSAGE_ScanStop);
	bufferDataUart.push_back(AssignData(OUTMESSAGE_ScanStop, 3));
	flag_selectmac     = false;
	flag_getpro_info   = false;
	flag_getpro_element= false;
	flag_provision     = false;
	flag_mac           = true;
	flag_check_select_mac  = false;
	flag_setpro  = false;
	flag_admitpro = false;
	flag_checkadmitpro = true;
	flag_set_type = false;
	flag_checkHB = false;
	flag_checkSaveGW = false;
	flag_checkTypeDEV = false;
	startCountHB = false;
	startCountSaveGW = false;
	startCountSetType = false;
	hasSelectMac = true;
	Pro_startCount = true;
}
void InitTimeoutRspProvision(){
	vrb_CheckingClock = true;
	gettimeofday(&vrcl_Start,NULL);
}
void StopCountCheckTimeout(){
	vrb_CheckingClock = false;
}

static uint16_t PRO_getSecondDay(){
	uint64_t second;
	struct timeval timeCurrent;
	gettimeofday(&timeCurrent,NULL);
	second = (timeCurrent.tv_sec);
	return second;
}
void* ProvisionThread(void *argv) {
	tmp = pthread_self();
	while (MODE_PROVISION) {
		if ((flag_done == true) || ((PRO_getSecondDay() - Timeout_CheckDataBuffer1) >= 15)) {
			scanNotFoundDev++;
			if (scanNotFoundDev == 2) {
				scanNotFoundDev = 0;
				MODE_PROVISION = false;
				bufferDataUart.push_back(AssignData(OUTMESSAGE_ScanStop, 3));
				gvrb_Provision = false;
				isProvision = false;
				slog_print(SLOG_INFO, 1, "<provision>Provision stop");
				mqtt_send(mosq,TP_PUB,"{\"CMD\":\"STOP\"}");
				flag_done = true;
				Pro_startCount = false;
				InitFalg();
			} else {
				flag_done = false;
				Pro_startCount = true;
				InitFalg();
				isProvision = true;
				bufferDataUart.push_back(AssignData(OUTMESSAGE_ScanStart, 3));
				slog_print(SLOG_INFO, 1, "<provision>SCAN");
				Timeout_CheckDataBuffer1 = PRO_getSecondDay();
				gvrb_Provision = true;
				flag_check_select_mac = true;
				InitTimeoutRspProvision();
			}
		}
		if ((flag_selectmac == true) && (flag_mac == true)) {
			flag_selectmac = false;
			flag_mac = false;
			hasSelectMac = false;
			bufferDataUart.push_back(AssignData(OUTMESSAGE_MACSelect, 9));
			slog_print(SLOG_INFO, 1, "<provision>SELECTMAC");
			sleep(1);
			bufferDataUart.push_back(AssignData(OUTMESSAGE_GetPro, 3));
			slog_print(SLOG_INFO, 1, "<provision>GETPRO");
			flag_checkadmitpro = false;
			InitTimeoutRspProvision();
		}
		if (flag_setpro == true) {
			flag_setpro = false;
			srand((int) time(0));
			int random1, random2;
			int i;
			for (i = 0; i < 16; i++) {
				random1 = rand() % 256;
				random2 = rand() % 256;
				setpro_internal[i + 3] = random1;
				admit_pro_internal[i + 5] = random2;
			}
			slog_print(SLOG_INFO, 1, "<provision>SETPRO....");
			bufferDataUart.push_back(AssignData(setpro_internal, 28));
			usleep(400000);
			slog_print(SLOG_INFO, 1, "<provision>ADMITPRO...");
			bufferDataUart.push_back(AssignData(admit_pro_internal, 21));
			sprintf((char*) PRO_deviceKey,
					"%02x%02x%02x%02x-%02x%02x-%02x%02x-%02x%02x-%02x%02x%02x%02x%02x%02x",
					admit_pro_internal[5], admit_pro_internal[6],
					admit_pro_internal[7], admit_pro_internal[8],
					admit_pro_internal[9], admit_pro_internal[10],
					admit_pro_internal[11], admit_pro_internal[12],
					admit_pro_internal[13], admit_pro_internal[14],
					admit_pro_internal[15], admit_pro_internal[16],
					admit_pro_internal[17], admit_pro_internal[18],
					admit_pro_internal[19], admit_pro_internal[20]);
			flag_checkadmitpro = true;
			InitTimeoutRspProvision();
		}
		if (flag_admitpro == true && flag_checkadmitpro == true) {
			flag_admitpro = false;
			flag_checkadmitpro = false;
			slog_print(SLOG_INFO, 1, "<provision>GETPRO");
			bufferDataUart.push_back(AssignData(OUTMESSAGE_GetPro, 3));
			InitTimeoutRspProvision();
		}
		if ((flag_getpro_info == true) /*&& (flag_getpro_element == true)*/) {
			flag_getpro_info = false;
			flag_getpro_element = false;
			bufferDataUart.push_back(AssignData(OUTMESSAGE_Provision, 28));
			slog_print(SLOG_INFO, 1, "<provision>PROVISION");
			InitTimeoutRspProvision();
		}
		if (flag_provision == true) {
			flag_provision = false;
			bufferDataUart.push_back(AssignData(OUTMESSAGE_BindingALl, 22));
			slog_print(SLOG_INFO, 1, "<provision>BINDING ALL");
			flag_set_type = false;
			InitTimeoutRspProvision();
		}
		if (flag_set_type == true) {
			flag_set_type = false;
			Function_Vendor(HCI_CMD_GATEWAY_CMD, SaveGateway_vendor_typedef,
					adr_heartbeat, NULL16,
					NULL8, NULL8, NULL8, NULL16, NULL16, NULL16, NULL16, NULL16,
					NULL16, NULL16,
					NULL8, NULL8, NULL8, NULL8, NULL16, 17);
			flag_checkSaveGW = false;
			startCountSaveGW = true;
			startP = clock();
		}
		if (startCountSaveGW && !flag_checkSaveGW && ((clock() - startP) >= vrTimeout)) {
			startCountSaveGW = false;
			Function_Vendor(HCI_CMD_GATEWAY_CMD, SaveGateway_vendor_typedef,
					adr_heartbeat, NULL16,
					NULL8, NULL8, NULL8, NULL16, NULL16, NULL16, NULL16, NULL16,
					NULL16, NULL16,
					NULL8, NULL8, NULL8, NULL8, NULL16, 17);
		}
		if (flag_checkSaveGW) {
			flag_checkSaveGW = false;
			startCountSaveGW = false;
			Function_Vendor(HCI_CMD_GATEWAY_CMD, AskTypeDevice_vendor_typedef,
					adr_heartbeat, NULL16,
					NULL8, NULL8, NULL8, NULL16, NULL16, NULL16, NULL16, NULL16,
					NULL16, NULL16, NULL8,
					NULL8, NULL8, NULL8, NULL16, 17);
			flag_checkTypeDEV = false;
			startP = clock();
			startCountSetType = true;
		}
		if (((clock() - startP) >= vrTimeout) && !flag_checkTypeDEV && startCountSetType) {
			startCountSetType = false;
			isSetTypeDev = true;
			Function_Vendor(HCI_CMD_GATEWAY_CMD, AskTypeDevice_vendor_typedef,
					adr_heartbeat, NULL16,
					NULL8, NULL8, NULL8, NULL16, NULL16, NULL16, NULL16, NULL16,
					NULL16, NULL16, NULL8,
					NULL8, NULL8, NULL8, NULL16, 17);
			endP = clock();
		}
		if ((flag_checkTypeDEV) || (!flag_checkTypeDEV && ((clock() - endP) >= 2 * vrTimeout) && isSetTypeDev)) {
			isSetTypeDev = false;
			InitFalg();
			flag_checkHB = false;
			flag_done = true;
			flag_mac = true;
		}
		if (vrb_CheckingClock) {
			gettimeofday(&vrcl_End, NULL);
			long seconds = (vrcl_End.tv_sec - vrcl_Start.tv_sec);
			long micros = ((seconds * 1000000) + vrcl_End.tv_usec) - (vrcl_Start.tv_usec);
			if (seconds >= 40) {
				slog_print(SLOG_INFO, 1, "<provision>TIMEOUT PROVISION");
				vrb_CheckingClock = false;
				flag_done = false;
				Pro_startCount = true;
				InitFalg();
				bufferDataUart.push_back(AssignData(OUTMESSAGE_ScanStart, 3));
				gvrb_Provision = true;
				slog_print(SLOG_INFO, 1, "<provision>SCAN");
				flag_check_select_mac = true;
			}
		}
	}
	return NULL;
}


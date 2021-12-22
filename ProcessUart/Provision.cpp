/*
 *Provision.c
 */
#include "Provision.hpp"
#include "../logging/slog.h"
#include "../Include/Include.hpp"
#include "../BuildCmdUart/BuildCmdUart.hpp"
#include "../Mqtt/Mqtt.hpp"

#include <sys/time.h>

#define TIMEOUT_FINDDEV		6
#define TIMEOUT_PRO			40
#define TIMEOUT_TYPE		1

uint8_t OUTMESSAGE_ScanStop[3]     = {0xE9, 0xFF, 0x01};
uint8_t OUTMESSAGE_ScanStart[3]    = {0xE9, 0xFF, 0x00};
uint8_t OUTMESSAGE_MACSelect[9]    = {0xE9, 0xFF, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
uint8_t OUTMESSAGE_GetPro[3] 	   = {0xE9, 0xFF, 0x0C};
uint8_t OUTMESSAGE_Provision[28]   = {0};
uint8_t OUTMESSAGE_BindingALl[22]  = {0xe9, 0xff, 0x0b, 0x00, 0x00, 0x00, 0x60, 0x96, 0x47, 0x71, 0x73, 0x4f, 0xbd, 0x76, 0xe3, 0xb4, 0x05, 0x19, 0xd1, 0xd9, 0x4a, 0x48};

uint8_t setpro_internal[]       =   {0xe9, 0xff, 0x09, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x11, 0x22, 0x33, 0x44, 0x01, 0x00};
uint8_t admit_pro_internal[]    =   {0xe9, 0xff, 0x0d, 0x01, 0x00, 0xff, 0xfb, 0xeb, 0xbf, 0xea, 0x06, 0x09, 0x00, 0x52, 0x90, 0x49, 0xf1, 0xf1, 0xbb, 0xe9, 0xeb};// trả về unicast tiếp theo của con đèn cần thêm vào
uint8_t reset_GW[]				=	{0xe9, 0xff, 0x02};

uint8_t PRO_deviceKey[37] = {0};
uint8_t PRO_netKey[37] = {0};
uint8_t PRO_appKey[37] = {0};
uint8_t PRO_uuid[37] = {0};
uint8_t PRO_mac[24] = {0};

statePro_t stateProvision = statePro_scan;
unsigned int adr_Provision = 0;
pthread_t vrpth_Pro = pthread_self();
static time_t timeCurrent, timeLast;

static void Pro_Scan(void) {
	gvrb_Provision = true;
	bufferDataUart.push_back(AssignData(OUTMESSAGE_ScanStart, 3));
	slog_print(SLOG_INFO, 1, "<provision>scan");
	stateProvision = statePro_findDev;
	timeCurrent = time(NULL);
}
static void Pro_Stop(void) {
	gvrb_Provision = false;
	bufferDataUart.push_back(AssignData(OUTMESSAGE_ScanStop, 3));
	slog_print(SLOG_INFO, 1, "<provision>stop");
	mqtt_send(mosq,TP_PUB,"{\"CMD\":\"STOP\"}");
	pthread_cancel(vrpth_Pro);
}

static void Pro_SelectMac(void) {
	bufferDataUart.push_back(AssignData(OUTMESSAGE_MACSelect, 9));
	slog_print(SLOG_INFO, 1, "<provision>select mac");
	sleep(1);
	stateProvision = statePro_getPro;
	timeCurrent = time(NULL);
}
static void Pro_GetPro(void) {
	bufferDataUart.push_back(AssignData(OUTMESSAGE_GetPro, 3));
	slog_print(SLOG_INFO, 1, "<provision>getpro");
	stateProvision = statePro_timeoutPro;
	timeCurrent = time(NULL);
}
static void Pro_SetPro(void) {
	srand((int) time(0));
	int random1, random2;
	int i;
	for (i = 0; i < 16; i++) {
		random1 = rand() % 256;
		random2 = rand() % 256;
		setpro_internal[i + 3] = random1;
		admit_pro_internal[i + 5] = random2;
	}
	bufferDataUart.push_back(AssignData(setpro_internal, 28));
	slog_print(SLOG_INFO, 1, "<probision>setpro");
	usleep(400000);
	bufferDataUart.push_back(AssignData(admit_pro_internal, 21));
	slog_print(SLOG_INFO, 1, "<probision>admitpro");

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
	stateProvision = statePro_getPro;
}
static void Pro_Provision(void) {
	bufferDataUart.push_back(AssignData(OUTMESSAGE_Provision, 28));
	slog_print(SLOG_INFO, 1, "<provision>provision");
	stateProvision = statePro_timeoutPro;
	timeCurrent = time(NULL);
}
static void Pro_BindingAll(void) {
	bufferDataUart.push_back(AssignData(OUTMESSAGE_BindingALl, 22));
	slog_print(SLOG_INFO, 1, "<provision>bindingAll");
	stateProvision = statePro_timeoutPro;
	timeCurrent = time(NULL);
}
static void Pro_SaveGate(void) {
	Function_Vendor(HCI_CMD_GATEWAY_CMD, SaveGateway_vendor_typedef,
			adr_Provision, NULL16, NULL8, NULL8, NULL8, NULL16, NULL16,
			NULL16, NULL16, NULL16, NULL16, NULL16, NULL8, NULL8, NULL8,
			NULL8, NULL16, 17);
	stateProvision = statePro_timeoutPro;
	timeCurrent = time(NULL);
}
static void Pro_TypeDev(void) {
	Function_Vendor(HCI_CMD_GATEWAY_CMD, AskTypeDevice_vendor_typedef,
			adr_Provision, NULL16, NULL8, NULL8, NULL8, NULL16, NULL16,
			NULL16, NULL16, NULL16, NULL16, NULL16, NULL8, NULL8, NULL8,
			NULL8, NULL16, 17);
	stateProvision = statePro_timeoutPro;
	timeCurrent = time(NULL);
}

static void TimeoutPro(void) {
	while (stateProvision == statePro_timeoutPro) {
		timeLast = time(NULL);
		if ((timeLast - timeCurrent) >= TIMEOUT_PRO) {
			timeCurrent = timeLast;
			stateProvision = statePro_scan;
		}
		usleep(5000);
	}
}
static void FinddDev(){
	while (stateProvision == statePro_findDev) {
		timeLast = time(NULL);
		if ((timeLast - timeCurrent) >= TIMEOUT_FINDDEV) {
			timeCurrent = timeLast;
			stateProvision = statePro_stop;
		}
		usleep(5000);
	}
}
static void Pro_null(){
}

void (*state_Table[])(void) = { Pro_Scan, Pro_Stop, Pro_SelectMac, Pro_GetPro,
		Pro_SetPro, Pro_Provision, Pro_BindingAll, Pro_SaveGate,
		Pro_TypeDev, TimeoutPro, FinddDev, Pro_null};

static statePro_t stateCurrent, stateLast;
void *Pro_Thread(void *argv){
	stateCurrent = statePro_null;
	while(MODE_PROVISION){
		stateLast = stateProvision;
		if(stateLast != stateCurrent){
			state_Table[stateProvision]();
			stateCurrent = stateLast;
		}
		usleep(1000);
	}
	return NULL;
}




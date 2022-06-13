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

using namespace std;
using namespace rapidjson;
bool provisionSuccess = false;

uint8_t OUTMESSAGE_ScanStop[3]     = {0xE9, 0xFF, 0x01};
uint8_t OUTMESSAGE_ScanStart[3]    = {0xE9, 0xFF, 0x00};
uint8_t OUTMESSAGE_MACSelect[9]    = {0xE9, 0xFF, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
uint8_t OUTMESSAGE_GetPro[3] 	   = {0xE9, 0xFF, 0x0C};
uint8_t OUTMESSAGE_Provision[28]   = {0};
uint8_t OUTMESSAGE_BindingALl[22]  = {0xe9, 0xff, 0x0b, 0x00, 0x00, 0x00, 0x60, 0x96, 0x47, 0x71, 0x73, 0x4f, 0xbd, 0x76, 0xe3, 0xb4, 0x05, 0x19, 0xd1, 0xd9, 0x4a, 0x48};

uint8_t setpro_internal[]       =   {0xe9, 0xff, 0x09, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x11, 0x22, 0x33, 0x44, 0x01, 0x00};

// trả về unicast tiếp theo của con đèn cần thêm vào
uint8_t admit_pro_internal[]    =   {0xe9, 0xff, 0x0d, 0x01, 0x00, 0xff, 0xfb, 0xeb, 0xbf, 0xea, 0x06, 0x09, 0x00, 0x52, 0x90, 0x49, 0xf1, 0xf1, 0xbb, 0xe9, 0xeb};

// Neu provision den loi thi reset unicast de gw dam bao so luong thiet bi quan ly
uint8_t reset_Node[]     		= 	{0xe8, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x49};

uint8_t PRO_deviceKeyGw[37] = {0};
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
	string s = "{\"CMD\":\"STOP\"}";
	slog_info("<mqtt>send: %s", s.c_str());
	Data2BufferSendMqtt(s);
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
	uint16_t adr_gw = (setpro_internal[26] | setpro_internal[27] << 8);
	bufferDataUart.push_back(AssignData(setpro_internal, 28));
	slog_print(SLOG_INFO, 1, "<probision>setpro");
	usleep(400000);
	bufferDataUart.push_back(AssignData(admit_pro_internal, 21));
	slog_print(SLOG_INFO, 1, "<probision>admitpro");

	sprintf((char*) PRO_deviceKeyGw,
			"%02x%02x%02x%02x-%02x%02x-%02x%02x-%02x%02x-%02x%02x%02x%02x%02x%02x",
			admit_pro_internal[5], admit_pro_internal[6],
			admit_pro_internal[7], admit_pro_internal[8],
			admit_pro_internal[9], admit_pro_internal[10],
			admit_pro_internal[11], admit_pro_internal[12],
			admit_pro_internal[13], admit_pro_internal[14],
			admit_pro_internal[15], admit_pro_internal[16],
			admit_pro_internal[17], admit_pro_internal[18],
			admit_pro_internal[19], admit_pro_internal[20]);
	uint8_t uuid_gw[37] = {0};
	sprintf((char*) uuid_gw,
		"%02x%02x%02x%02x-%02x%02x-%02x%02x-%02x%02x-%02x%02x%02x%02x%02x%02x",
		admit_pro_internal[5],admit_pro_internal[6],
		admit_pro_internal[7],admit_pro_internal[8],
		admit_pro_internal[9],admit_pro_internal[10],
		admit_pro_internal[11],0,0,0,0,0,0,0,0,0);
	StringBuffer dataMqtt1;
	Writer<StringBuffer> json1(dataMqtt1);
	json1.StartObject();
		json1.Key("CMD");json1.String("NEW_DEVICE");
		json1.Key("DATA");
		json1.StartObject();
			json1.Key("DEVICE_UNICAST_ID");json1.Int(adr_gw);
			json1.Key("DEVICE_ID");json1.String((char *)uuid_gw);
			json1.Key("DEVICE_TYPE_ID");json1.Int(0);
			json1.Key("MAC_ADDRESS");json1.String("00:00:00:00:00:00");
			json1.Key("FIRMWARE_VERSION");json1.String("1.0");
			json1.Key("DEVICE_KEY");json1.String((char *)PRO_deviceKeyGw);
			json1.Key("NET_KEY");json1.String("");
			json1.Key("APP_KEY");json1.String("");
		json1.EndObject();
	json1.EndObject();

	//	cout << dataMqtt.GetString() << endl;
	string s1 = dataMqtt1.GetString();
	slog_info("<mqtt>send: %s", s1.c_str());
	Data2BufferSendMqtt(s1);
	stateProvision = statePro_getPro;
}
static void Pro_Provision(void) {
	bufferDataUart.push_back(AssignData(OUTMESSAGE_Provision, 28));
	slog_print(SLOG_INFO, 1, "<provision>provision");
	stateProvision = statePro_timeoutPro;
	provisionSuccess = false;
	timeCurrent = time(NULL);
}
static void Pro_BindingAll(void) {
	bufferDataUart.push_back(AssignData(OUTMESSAGE_BindingALl, 22));
	slog_print(SLOG_INFO, 1, "<provision>bindingAll");
	stateProvision = statePro_timeoutPro;
	timeCurrent = time(NULL);
}
static void Pro_SaveGate(void) {
	sleep(1);
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
			reset_Node[8] = adr_Provision & 0xFF;
			reset_Node[9] = (adr_Provision >> 8) & 0xFF;
			bufferDataUart.push_back(AssignData(reset_Node, 12));
			sleep(1);
			stateProvision = statePro_scan;
		}
		usleep(5000);
	}
}
static void FinddDev(void){
	while (stateProvision == statePro_findDev) {
		timeLast = time(NULL);
		if ((timeLast - timeCurrent) >= TIMEOUT_FINDDEV) {
			timeCurrent = timeLast;
			stateProvision = statePro_stop;
			provisionSuccess = true;
		}
		usleep(5000);
	}
}
static void Pro_null(void){
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
	pthread_cancel(pthread_self());
	return NULL;
}




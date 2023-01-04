/*
 *Provision.c
 */
#include "Provision.hpp"
#include "../logging/slog.h"
#include "../Include/Include.hpp"
#include "../BuildCmdUart/BuildCmdUart.hpp"
#include "../Mqtt/Mqtt.hpp"
#include <sqlite3.h>
#include "AES.h"

#include <sys/time.h>

#define TIMEOUT_FINDDEV		6
#define TIMEOUT_PRO			40
#define TIMEOUT_TYPE		1

using namespace std;
using namespace rapidjson;
bool provisionSuccess = false;
bool gwBleAction = false;
static string appKeyBle;

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

//save gateway
uint8_t save_gw[]				=	{0xe8, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xe0, 0x11, 0x02, 0xe1, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
uint8_t request_type[]			= 	{0xe8, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xe0, 0x11, 0x02, 0xe1, 0x00, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

//Ral
int plainLength = 32;
static uint8_t keyAes[] 		= 	{0x44, 0x69, 0x67, 0x69, 0x74, 0x61, 0x6c, 0x40, 0x32, 0x38, 0x31, 0x31, 0x32, 0x38, 0x30, 0x34};
uint8_t plaintext[]				=	{0x24, 0x02, 0x28, 0x04, 0x28, 0x11, 0x20, 0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

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
uint16_t adrMax = 0;

template<typename T>
string ToString(const T &value) {
	std::ostringstream oss;
	oss << value;
	return oss.str();
}

static int getAdrMax(void *data, int argc, char **argv, char **azColName)
{
	if(argc > 0)
	{
		for (int i = 0; i < argc; i++) {
			if ((const char *)argv[0] != NULL) {
				adrMax = atoi((const char *)argv[0]);
			}
		}
	}
	return 0;
}

static int getAppKeyBLE(void *data, int argc, char **argv, char **azColName)
{
	if (argc > 0) {
		for(int i = 0; i < argc; i++)
		{
			appKeyBle = ToString((char *)argv[i]);
		}
	}
	return 0;
}

static void Pro_GetDb(string cmd, string sql) {
	sqlite3 *db;
	int exit = 0;
	do {
		exit = sqlite3_open("/root/rd.Sqlite", &db);
	} while (exit != SQLITE_OK);
	char *messaggeError;
	if (cmd.compare("getAddMax") == 0)
	{
		while (sqlite3_exec(db, sql.c_str(), getAdrMax, 0, &messaggeError) != SQLITE_OK) {
			puts(messaggeError);
			sqlite3_free(messaggeError);
			usleep(100000);
		}
	}
	else if (cmd.compare("getAppKey") == 0)
	{
		while (sqlite3_exec(db, sql.c_str(), getAppKeyBLE, 0, &messaggeError) != SQLITE_OK) {
			puts(messaggeError);
			sqlite3_free(messaggeError);
			usleep(100000);
		}
	}
	sqlite3_close(db);
}

static void Pro_Scan(void) {
	gvrb_Provision = true;
	bufferDataUart.push_back(AssignData(OUTMESSAGE_ScanStart, 3, 10));
	slog_print(SLOG_INFO, 1, "<provision>scan");
	stateProvision = statePro_findDev;
	timeCurrent = time(NULL);
}
static void Pro_Stop(void) {
	gvrb_Provision = false;
	bufferDataUart.push_back(AssignData(OUTMESSAGE_ScanStop, 3, 10));
	slog_print(SLOG_INFO, 1, "<provision>stop");
	string s = "{\"CMD\":\"STOP\"}";
	slog_info("<mqtt>send: %s", s.c_str());
	Data2BufferSendMqtt(s);
	pthread_cancel(vrpth_Pro);
}
static void Pro_SelectMac(void) {
	bufferDataUart.push_back(AssignData(OUTMESSAGE_MACSelect, 9, 10));
	slog_print(SLOG_INFO, 1, "<provision>select mac");
	sleep(1);
	stateProvision = statePro_getPro;
	timeCurrent = time(NULL);
}
static void Pro_GetPro(void) {
	bufferDataUart.push_back(AssignData(OUTMESSAGE_GetPro, 3, 10));
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
	bufferDataUart.push_back(AssignData(setpro_internal, 28, 10));
	slog_print(SLOG_INFO, 1, "<probision>setpro");
	usleep(400000);
	bufferDataUart.push_back(AssignData(admit_pro_internal, 21, 10));
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
	bufferDataUart.push_back(AssignData(OUTMESSAGE_Provision, 28, 10));
	slog_print(SLOG_INFO, 1, "<provision>provision");
	stateProvision = statePro_timeoutPro;
	provisionSuccess = false;
	timeCurrent = time(NULL);
}
static void Pro_BindingAll(void) {
	bufferDataUart.push_back(AssignData(OUTMESSAGE_BindingALl, 22, 10));
	slog_print(SLOG_INFO, 1, "<provision>bindingAll");
	stateProvision = statePro_timeoutPro;
	timeCurrent = time(NULL);
}
static void Pro_SaveGate(void) {
	save_gw[8] = adr_Provision;
	save_gw[9] = (adr_Provision >> 8) & 0xFF;
	bufferDataUart.push_back(AssignData(save_gw, 23, 10));
	stateProvision = statePro_timeoutPro;
	timeCurrent = time(NULL);
}
static void Pro_TypeDev(void) {
	request_type[8] = adr_Provision;
	request_type[9] = (adr_Provision >> 8) & 0xFF;
	plaintext[14] = adr_Provision;
	plaintext[15] = (adr_Provision >> 8) & 0xFF;
	AES aes(AESKeyLength::AES_128);
	unsigned char *out = aes.EncryptECB(plaintext, plainLength, keyAes);
	for(int j = 0; j < 32; j ++){
		printf("%02x ",out[j]);
	}
	printf("\n");
	for (int i = 0; i < 6; i++) {
		request_type[i+17] = out[i+10];
	}
	bufferDataUart.push_back(AssignData(request_type, 23, 10));
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
			bufferDataUart.push_back(AssignData(reset_Node, 12, 10));
			sleep(1);
			stateProvision = statePro_scan;
		}
		usleep(5000);
	}
}
static void FindDev(void){
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
static void Pro_ResetNode(void){
	reset_Node[8] = adr_Provision & 0xFF;
	reset_Node[9] = (adr_Provision >> 8) & 0xFF;
	bufferDataUart.push_back(AssignData(reset_Node, 12, 10));
	stateProvision = statePro_scan;
}

static void Pro_GetInfoMeshBle() {
	//Get app key
	string sql = "SELECT AppKey FROM Device Where DeviceId NOT NULL;";
	Pro_GetDb("getAppKey",sql);
	slog_info("Appkey: %s", appKeyBle.c_str());
	if (appKeyBle.compare("") != 0)
	{
		appKeyBle.erase(appKeyBle.begin()+8, appKeyBle.begin()+9);
		appKeyBle.erase(appKeyBle.begin()+12, appKeyBle.begin()+13);
		appKeyBle.erase(appKeyBle.begin()+16, appKeyBle.begin()+17);
		appKeyBle.erase(appKeyBle.begin()+20, appKeyBle.begin()+21);
		char * ak = new char[appKeyBle.length()+1];
		strcpy(ak,appKeyBle.c_str());
		uint8_t temp[17] = {0};
		for(int i =0; i < 16; i++)
		{
			sscanf((char *)ak + i*2,"%2x" ,&temp[i]);
			OUTMESSAGE_BindingALl[i+6] = temp[i];
		}
	}

	//Get Device Unicast Max
	string sql1 = "SELECT Max(DeviceunicastId) FROM Device Where DeviceId NOT NULL;";
	Pro_GetDb("getAddMax",sql1);
	slog_info("Device unicast max: %d", adrMax);
}

void (*state_Table[])(void) = { Pro_Scan, Pro_Stop, Pro_SelectMac, Pro_GetPro,
		Pro_SetPro, Pro_Provision, Pro_BindingAll, Pro_SaveGate,
		Pro_TypeDev, TimeoutPro, FindDev, Pro_ResetNode};

static statePro_t stateCurrent, stateLast;
void *Pro_Thread(void *argv)
{
	stateCurrent = statePro_null;
	Pro_GetInfoMeshBle();
	while(MODE_PROVISION)
	{
		stateLast = stateProvision;
		if(stateLast != stateCurrent)
		{
			state_Table[stateProvision]();
			stateCurrent = stateLast;
		}
		usleep(1000);
	}
	pthread_cancel(pthread_self());
	return NULL;
}




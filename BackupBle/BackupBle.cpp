/*
 * BackupBle.cpp
 *
 *  Created on: May 12, 2022
 *      Author: anh
 */

#include "BackupBle.hpp"
#include "../logging/slog.h"

using namespace std;
using namespace rapidjson;

#define BACKUP_MAXDEV   500

static uint16_t BACKUP_countDev = 0;
static uint8_t BACKUP_countGw = 0;
static string BACKUP_appKeyDb;
static string BACKUP_netKeyDb;

static uint8_t BACKUP_resetGw[3]   			= {0xe9, 0xff, 0x02};
static uint8_t BACKUP_getInfoGw[4] 			= {0xe9, 0xff, 0x16, 0x00};
static uint8_t BACKUP_getPro[3]    			= {0xe9, 0xff, 0x0c};
static uint8_t BACKUP_updateDevKeyDev[23] 	= {0};
static uint8_t BACKUP_updateDevKeyGw[23] 	= {0};
static uint8_t BACKUP_netKey[28] 			= {0};
static uint8_t BACKUP_devKeygw[21]			= {0};
static uint8_t BACKUP_appKey[22]			= {0};
static uint8_t BACKUP_deviceUnicastId[28]   = {0};



infoDev_t BACKUP_infoGw;
infoDev_t BACKUP_listDev[BACKUP_MAXDEV];

int BACKUP_UpdateListDev(void *data, int argc, char **argv, char **azColName) {
	if(argc) {
			BACKUP_listDev[BACKUP_countDev].adr 	= atoi((const char*) argv[0]);
			BACKUP_listDev[BACKUP_countDev].devKey 	= string((char *)argv[1]);
			BACKUP_countDev++;
	}
	return 0;
}

int BACKUP_UpdateGw(void *data, int argc, char **argv, char **azColName) {
	if(argc) {
			BACKUP_infoGw.adr 	= atoi((const char*) argv[0]);
			BACKUP_infoGw.devKey 	= string((char *)argv[1]);
			BACKUP_countGw++;
	}
	return 0;
}


int BACKUP_getAppKey(void *data, int argc, char **argv, char **azColName) {
	if(argc) {
		BACKUP_appKeyDb = string((char*)argv[0]);
	}
	return 0;
}

int BACKUP_getNetKey(void *data, int argc, char **argv, char **azColName) {
	if(argc) {
		BACKUP_netKeyDb = string((char*)argv[0]);
	}
	return 0;
}

/*
 * OpenDB(): open and query to db
 */
int BACKUP_OpenDB(char * db, string sqlQuery, type_get_db type){
	sqlite3 *DB;
	char * msgError;
	int open = 0;
	do{
		open = sqlite3_open(db,&DB);
	}while(open != SQLITE_OK);

	if(type == inforGw) {
		if(sqlite3_exec(DB, sqlQuery.c_str(), BACKUP_UpdateGw, 0, &msgError) != SQLITE_OK){
		}
	} else if (type == inforDev) {
		if(sqlite3_exec(DB, sqlQuery.c_str(), BACKUP_UpdateListDev, 0, &msgError) != SQLITE_OK){
		}
	} else if (type == infoAppKey) {
		if(sqlite3_exec(DB, sqlQuery.c_str(), BACKUP_getAppKey, 0, &msgError) != SQLITE_OK){
		}
	} else if (type == infoNetKey) {
		if(sqlite3_exec(DB, sqlQuery.c_str(), BACKUP_getNetKey, 0, &msgError) != SQLITE_OK){
		}
	}

	sqlite3_close(DB);
	return 0;
}

void BACKUP_updateNetKey(string netkey, infoDev_t gw, uint32_t index) {
//	string getNetKey = "SELECT DISTINCT NetKey from Device;";
//	BACKUP_OpenDB(db,getNetKey, infoNetKey);
	netkey.erase(netkey.begin()+8, netkey.begin()+9);
	netkey.erase(netkey.begin()+12, netkey.begin()+13);
	netkey.erase(netkey.begin()+16, netkey.begin()+17);
	netkey.erase(netkey.begin()+20, netkey.begin()+21);

	BACKUP_netKey[0] = 0xe9;
	BACKUP_netKey[1] = 0xff;
	BACKUP_netKey[2] = 0x09;

	for(int i = 0; i < 16; i++) {
		sscanf(netkey.c_str() + i*2, "%2x", &BACKUP_netKey[i+3]);
	}
	BACKUP_netKey[19] = 0;
	BACKUP_netKey[20] = 0;
	BACKUP_netKey[21] = 0;
	BACKUP_netKey[22] = (index >> 24) & 0xFF;
	BACKUP_netKey[23] = (index >> 16) & 0xFF;
	BACKUP_netKey[24] = (index >> 8) & 0xFF;
	BACKUP_netKey[25] = index & 0xFF;
	BACKUP_netKey[26] = gw.adr & 0xFF;
	BACKUP_netKey[27] = (gw.adr >> 8) & 0xFF;
//	printf("update netkey: ");
//	for(int j = 0; j < 28; j++){
//		printf("%2x-",BACKUP_netKey[j]);
//	}
//	printf("\n");
	bufferDataUart.push_back(AssignData(BACKUP_netKey, 28));
	sleep(3);
}

void BACKUP_updateNewDevKey(infoDev_t gw) {
	BACKUP_devKeygw[0] = 0xe9;
	BACKUP_devKeygw[1] = 0xff;
	BACKUP_devKeygw[2] = 0x0d;
	BACKUP_devKeygw[3] = gw.adr & 0xFF;
	BACKUP_devKeygw[4] = (gw.adr >> 8 ) & 0xFF;
	srand((int)time(0));
	int random1;
	for (int i = 0; i < 16; i++) {
		random1 = rand()%256;
		BACKUP_devKeygw[i+5] = random1;
	}
//	printf("update new devKey:");
//	for(int j = 0; j < 21; j++) {
//		printf("%2x-",BACKUP_devKeygw[j]);
//	}
//	printf("\n");
	bufferDataUart.push_back(AssignData(BACKUP_devKeygw, 21));
	sleep(3);
}

void BACKUP_updateAppKey(string appkey) {
//	string getAppKey = "SELECT DISTINCT AppKey from Device;";
//	BACKUP_OpenDB(db,getAppKey, infoAppKey);
	appkey.erase(appkey.begin()+8, appkey.begin()+9);
	appkey.erase(appkey.begin()+12, appkey.begin()+13);
	appkey.erase(appkey.begin()+16, appkey.begin()+17);
	appkey.erase(appkey.begin()+20, appkey.begin()+21);

	BACKUP_appKey[0] = 0xe9;
	BACKUP_appKey[1] = 0xff;
	BACKUP_appKey[2] = 0x0b;
	BACKUP_appKey[3] = 0x01;
	BACKUP_appKey[4] = 0x00;
	BACKUP_appKey[5] = 0x00;

	for(int i = 0; i < 16; i++) {
		sscanf(appkey.c_str() + i*2, "%2x", &BACKUP_appKey[i+6]);
	}
//	printf("update appkey: ");
//	for(int j = 0; j < 22; j++){
//		printf("%2x-",BACKUP_appKey[j]);
//	}
//	printf("\n");
	bufferDataUart.push_back(AssignData(BACKUP_appKey, 22));
	sleep(3);
}

void BACKUP_gw(infoDev_t gw) {
//	string getInfoGw = "SELECT DeviceUnicastId,DeviceKey FROM Device WHERE CategoryId = 0 AND DeviceId IS NOT NULL AND DeviceUnicastId IS NOT NULL AND DeviceKey IS NOT NULL;";
//	BACKUP_countGw = 0;
//	BACKUP_OpenDB(db, getInfoGw,inforGw);
//	if(BACKUP_countGw){

		gw.devKey.erase(gw.devKey.begin()+8, gw.devKey.begin()+9);
		gw.devKey.erase(gw.devKey.begin()+12, gw.devKey.begin()+13);
		gw.devKey.erase(gw.devKey.begin()+16, gw.devKey.begin()+17);
		gw.devKey.erase(gw.devKey.begin()+20, gw.devKey.begin()+21);

		BACKUP_updateDevKeyGw[0] = 0xe9;
		BACKUP_updateDevKeyGw[1] = 0xff;
		BACKUP_updateDevKeyGw[2] = 0x12;
		BACKUP_updateDevKeyGw[3] = gw.adr;
		BACKUP_updateDevKeyGw[4] = gw.adr >> 8;
		BACKUP_updateDevKeyGw[5] = 0x01;
		BACKUP_updateDevKeyGw[6] = 0x00;
		for(int i = 0; i < 16; i++) {
			sscanf(gw.devKey.c_str() + i*2, "%2x", &BACKUP_updateDevKeyGw[i+7]);
		}
//		printf("update gw:");
//		for(int j = 0; j < 23; j++){
//			printf("%2x-",BACKUP_updateDevKeyGw[j]);
//		}
//		printf("\n");
		bufferDataUart.push_back(AssignData(BACKUP_updateDevKeyGw, 23));
		sleep(3);
//	}
}

void BACKUP_dev(vector<infoDev_t> listDev) {
//	string getInfoDev = "SELECT DeviceUnicastId,DeviceKey FROM Device WHERE CategoryId != 0 AND DeviceId IS NOT NULL AND DeviceUnicastId IS NOT NULL AND DeviceKey IS NOT NULL;";
//	BACKUP_countDev = 0;
//	BACKUP_OpenDB(db,getInfoDev,inforDev);
	vector<infoDev_t>::iterator iter_name;
	for (iter_name = listDev.begin(); iter_name != listDev.end(); iter_name++)
	{
		iter_name->devKey.erase(iter_name->devKey.begin() + 8, iter_name->devKey.begin() + 9);
		iter_name->devKey.erase(iter_name->devKey.begin() + 12, iter_name->devKey.begin() + 13);
		iter_name->devKey.erase(iter_name->devKey.begin() + 16, iter_name->devKey.begin() + 17);
		iter_name->devKey.erase(iter_name->devKey.begin() + 20, iter_name->devKey.begin() + 21);
		BACKUP_updateDevKeyDev[0] = 0xe9;
		BACKUP_updateDevKeyDev[1] = 0xff;
		BACKUP_updateDevKeyDev[2] = 0x12;
		BACKUP_updateDevKeyDev[3] = iter_name->adr;
		BACKUP_updateDevKeyDev[4] = iter_name->adr >> 8;
		BACKUP_updateDevKeyDev[5] = 0x02;
		BACKUP_updateDevKeyDev[6] = 0x00;
		for(int n = 0; n < 16; n++) {
			sscanf(iter_name->devKey.c_str() + n*2, "%2x", &BACKUP_updateDevKeyDev[n+7]);
		}
//			printf("update device:");
//			for(int j = 0; j < 23; j++){
//				printf("%2x-",BACKUP_updateDevKeyDev[j]);
//			}
//			printf("\n");
		bufferDataUart.push_back(AssignData(BACKUP_updateDevKeyDev, 23));
		sleep(3);
	}
}

#define OFFSET			8
void BACKUP_DeviceUnicastIdMax(vector<infoDev_t> listDev){
	slog_print(SLOG_ERROR, 1, "tp1");
	uint16_t deviceUncastId = 0;
	vector<infoDev_t>::iterator iter_name;
	for(iter_name = listDev.begin(); iter_name != listDev.end(); iter_name++) {
		if (iter_name->adr > deviceUncastId) {
			deviceUncastId = iter_name->adr;
		}
	}
	slog_print(SLOG_ERROR, 1, "tp2");
	BACKUP_deviceUnicastId[0] = 0xe9;
	BACKUP_deviceUnicastId[1] = 0xff;
	BACKUP_deviceUnicastId[2] = 0x0a;
	for(int j = 0; j < 23 ; j++) {
		BACKUP_deviceUnicastId[3+j] = BACKUP_netKey[3+j];
	}
	BACKUP_deviceUnicastId[26] = (deviceUncastId + OFFSET) & 0xFF;
	BACKUP_deviceUnicastId[27] = ((deviceUncastId + OFFSET) >> 8) & 0xFF;
	slog_print(SLOG_ERROR, 1, "tp3");
	bufferDataUart.push_back(AssignData(BACKUP_deviceUnicastId, 28));
	slog_print(SLOG_ERROR, 1, "tp4");
	sleep(1);
}

void BACKUP_Init() {
	bufferDataUart.push_back(AssignData(BACKUP_resetGw, 3));
	sleep(8);
	bufferDataUart.push_back(AssignData(BACKUP_getInfoGw, 4));
	bufferDataUart.push_back(AssignData(BACKUP_getPro, 3));
}
void BACKUP2HC_callback(string db) {
	infoDev_t gw;
	vector<infoDev_t> listDev;

	BACKUP_Init();

	// backup devicekey gw
	string getInfoGw = "SELECT DeviceUnicastId,DeviceKey FROM Device WHERE CategoryId = 0 AND DeviceId IS NOT NULL AND DeviceUnicastId IS NOT NULL AND DeviceKey IS NOT NULL;";
	BACKUP_countGw = 0;
	BACKUP_OpenDB((char*)db.c_str(), getInfoGw, inforGw);
	if (BACKUP_countGw > 0){
		gw = BACKUP_infoGw;
		BACKUP_gw(gw);
	} else {
		slog_print(SLOG_WARN, 1, "<backup> don't have infomation gateway");
	}

	// backup devicekey device
	string getInfoDev = "SELECT DeviceUnicastId,DeviceKey FROM Device WHERE CategoryId != 0 AND DeviceId IS NOT NULL AND DeviceUnicastId IS NOT NULL AND DeviceKey IS NOT NULL;";
	BACKUP_countDev = 0;
	BACKUP_OpenDB((char*)db.c_str(),getInfoDev,inforDev);
	if (BACKUP_countDev > 0) {
		for(int i = 0; i < BACKUP_countDev; i++) {
			listDev.push_back(BACKUP_listDev[i]);
		}
		BACKUP_dev(listDev);
	} else {
		slog_print(SLOG_WARN, 1, "<backup> don't have any device");
	}
	string getNetKey = "SELECT DISTINCT NetKey from Device;";
	BACKUP_OpenDB((char*)db.c_str(),getNetKey, infoNetKey);
	string netkey = BACKUP_netKeyDb;
	if(BACKUP_netKeyDb.compare("null") != 0){
		BACKUP_updateNetKey(netkey, gw, 0x11223344);
	} else {
		slog_print(SLOG_WARN, 1, "<backup> don't have netkey");
	}
	BACKUP_DeviceUnicastIdMax(listDev);
	BACKUP_updateNewDevKey(gw);
	string getAppKey = "SELECT DISTINCT AppKey from Device;";
	BACKUP_OpenDB((char*)db.c_str(),getAppKey, infoAppKey);
	string appkey = BACKUP_appKeyDb;
	if (appkey.compare("null") != 0) {
		BACKUP_updateAppKey(appkey);
	}

}


/*
 * BackupBle.cpp
 *
 *  Created on: May 12, 2022
 *      Author: anh
 */

#include "BackupBle.hpp"

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
static uint8_t BACKUP_updateDevKeyDev[23] 		= {0};
static uint8_t BACKUP_updateDevKeyGw[23] 	= {0};
static uint8_t BACKUP_netKey[28] 			= {0};
static uint8_t BACKUP_devKeygw[21]			= {0};
static uint8_t BACKUP_appKey[22]			= {0};

typedef enum{
	inforGw,
	inforDev,
	infoAppKey,
	infoNetKey
}type_get_db;

typedef struct infoDev{
	uint16_t adr;
	string devKey;
}infoDev_t;

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
int BACKUP_OpenDB(string sqlQuery, type_get_db type){
	sqlite3 *DB;
	char * msgError;
	int open = 0;
	do{
		open = sqlite3_open("/root/rd.Sqlite",&DB);
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

void BACKUP_updateNetKey() {
	string getNetKey = "SELECT DISTINCT NetKey from Device;";
	BACKUP_OpenDB(getNetKey, infoNetKey);
	BACKUP_netKeyDb.erase(BACKUP_netKeyDb.begin()+8, BACKUP_netKeyDb.begin()+9);
	BACKUP_netKeyDb.erase(BACKUP_netKeyDb.begin()+12, BACKUP_netKeyDb.begin()+13);
	BACKUP_netKeyDb.erase(BACKUP_netKeyDb.begin()+16, BACKUP_netKeyDb.begin()+17);
	BACKUP_netKeyDb.erase(BACKUP_netKeyDb.begin()+20, BACKUP_netKeyDb.begin()+21);

	BACKUP_netKey[0] = 0xe9;
	BACKUP_netKey[1] = 0xff;
	BACKUP_netKey[2] = 0x09;

	for(int i = 0; i < 16; i++) {
		sscanf(BACKUP_netKeyDb.c_str() + i*2, "%2x", &BACKUP_netKey[i+3]);
	}
	BACKUP_netKey[19] = 0;
	BACKUP_netKey[20] = 0;
	BACKUP_netKey[21] = 0;
	BACKUP_netKey[22] = 0x11;
	BACKUP_netKey[23] = 0x22;
	BACKUP_netKey[24] = 0x33;
	BACKUP_netKey[25] = 0x44;
	BACKUP_netKey[26] = BACKUP_infoGw.adr & 0xFF;
	BACKUP_netKey[27] = (BACKUP_infoGw.adr >> 8) & 0xFF;
//	printf("update netkey: ");
//	for(int j = 0; j < 28; j++){
//		printf("%2x-",BACKUP_netKey[j]);
//	}
//	printf("\n");
	bufferDataUart.push_back(AssignData(BACKUP_netKey, 28));
	sleep(3);
}

void BACKUP_updateNewDevKey() {
	BACKUP_devKeygw[0] = 0xe9;
	BACKUP_devKeygw[1] = 0xff;
	BACKUP_devKeygw[2] = 0x0d;
	BACKUP_devKeygw[3] = BACKUP_infoGw.adr & 0xFF;
	BACKUP_devKeygw[4] = (BACKUP_infoGw.adr >> 8 ) & 0xFF;
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

void BACKUP_updateAppKey() {
	string getAppKey = "SELECT DISTINCT AppKey from Device;";
	BACKUP_OpenDB(getAppKey, infoAppKey);
	BACKUP_appKeyDb.erase(BACKUP_appKeyDb.begin()+8, BACKUP_appKeyDb.begin()+9);
	BACKUP_appKeyDb.erase(BACKUP_appKeyDb.begin()+12, BACKUP_appKeyDb.begin()+13);
	BACKUP_appKeyDb.erase(BACKUP_appKeyDb.begin()+16, BACKUP_appKeyDb.begin()+17);
	BACKUP_appKeyDb.erase(BACKUP_appKeyDb.begin()+20, BACKUP_appKeyDb.begin()+21);

	BACKUP_appKey[0] = 0xe9;
	BACKUP_appKey[1] = 0xff;
	BACKUP_appKey[2] = 0x0b;
	BACKUP_appKey[3] = 0x00;
	BACKUP_appKey[4] = 0x00;
	BACKUP_appKey[5] = 0x00;

	for(int i = 0; i < 16; i++) {
		sscanf(BACKUP_appKeyDb.c_str() + i*2, "%2x", &BACKUP_appKey[i+6]);
	}
//	printf("update appkey: ");
//	for(int j = 0; j < 22; j++){
//		printf("%2x-",BACKUP_appKey[j]);
//	}
//	printf("\n");
	bufferDataUart.push_back(AssignData(BACKUP_appKey, 22));
}

void BACKUP_gw() {
	string getInfoGw = "SELECT DeviceUnicastId,DeviceKey FROM Device WHERE CategoryId = 0 AND DeviceId IS NOT NULL AND DeviceUnicastId IS NOT NULL AND DeviceKey IS NOT NULL;";
	BACKUP_countGw = 0;
	BACKUP_OpenDB(getInfoGw,inforGw);
	if(BACKUP_countGw){
		BACKUP_infoGw.devKey.erase(BACKUP_infoGw.devKey.begin()+8, BACKUP_infoGw.devKey.begin()+9);
		BACKUP_infoGw.devKey.erase(BACKUP_infoGw.devKey.begin()+12, BACKUP_infoGw.devKey.begin()+13);
		BACKUP_infoGw.devKey.erase(BACKUP_infoGw.devKey.begin()+16, BACKUP_infoGw.devKey.begin()+17);
		BACKUP_infoGw.devKey.erase(BACKUP_infoGw.devKey.begin()+20, BACKUP_infoGw.devKey.begin()+21);

		BACKUP_updateDevKeyGw[0] = 0xe9;
		BACKUP_updateDevKeyGw[1] = 0xff;
		BACKUP_updateDevKeyGw[2] = 0x12;
		BACKUP_updateDevKeyGw[3] = BACKUP_infoGw.adr;
		BACKUP_updateDevKeyGw[4] = BACKUP_infoGw.adr >> 8;
		BACKUP_updateDevKeyGw[5] = 0x01;
		BACKUP_updateDevKeyGw[6] = 0x00;
		for(int i = 0; i < 16; i++) {
			sscanf(BACKUP_infoGw.devKey.c_str() + i*2, "%2x", &BACKUP_updateDevKeyGw[i+7]);
		}
//		printf("update gw:");
//		for(int j = 0; j < 23; j++){
//			printf("%2x-",BACKUP_updateDevKeyGw[j]);
//		}
//		printf("\n");
		bufferDataUart.push_back(AssignData(BACKUP_updateDevKeyGw, 23));
		sleep(3);
	}
}

void BACKUP_dev() {
	string getInfoDev = "SELECT DeviceUnicastId,DeviceKey FROM Device WHERE CategoryId != 0 AND DeviceId IS NOT NULL AND DeviceUnicastId IS NOT NULL AND DeviceKey IS NOT NULL;";
	BACKUP_countDev = 0;
	BACKUP_OpenDB(getInfoDev,inforDev);
	if(BACKUP_countDev > 0) {
		for(int i=0; i < BACKUP_countDev; i++) {
			BACKUP_listDev[i].devKey.erase(BACKUP_listDev[i].devKey.begin() + 8, BACKUP_listDev[i].devKey.begin() + 9);
			BACKUP_listDev[i].devKey.erase(BACKUP_listDev[i].devKey.begin() + 12, BACKUP_listDev[i].devKey.begin() + 13);
			BACKUP_listDev[i].devKey.erase(BACKUP_listDev[i].devKey.begin() + 16, BACKUP_listDev[i].devKey.begin() + 17);
			BACKUP_listDev[i].devKey.erase(BACKUP_listDev[i].devKey.begin() + 20, BACKUP_listDev[i].devKey.begin() + 21);
//			cout << ">>>>" << BACKUP_listDev[i].devKey << endl;
			BACKUP_updateDevKeyDev[0] = 0xe9;
			BACKUP_updateDevKeyDev[1] = 0xff;
			BACKUP_updateDevKeyDev[2] = 0x12;
			BACKUP_updateDevKeyDev[3] = BACKUP_listDev[i].adr;
			BACKUP_updateDevKeyDev[4] = BACKUP_listDev[i].adr >> 8;
			BACKUP_updateDevKeyDev[5] = 0x00;
			BACKUP_updateDevKeyDev[6] = 0x00;
			for(int n = 0; n < 16; n++) {
				sscanf(BACKUP_listDev[i].devKey.c_str() + n*2, "%2x", &BACKUP_updateDevKeyDev[n+7]);
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
}

void BACKUP_callback() {
	bufferDataUart.push_back(AssignData(BACKUP_resetGw, 3));
	sleep(8);
	bufferDataUart.push_back(AssignData(BACKUP_getInfoGw, 4));
	bufferDataUart.push_back(AssignData(BACKUP_getPro, 3));
	BACKUP_gw();
	BACKUP_dev();
	BACKUP_updateNetKey();
	BACKUP_updateNewDevKey();
	BACKUP_updateAppKey();
}


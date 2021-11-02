//============================================================================
// Name        : HC_UART_CPP.cpp
// Author      : anh
// Version     :
// Copyright   : Your copyright notice
// Description : Hello World in C++, Ansi-style
//============================================================================

#include "ProcessUart/GateInterface.hpp"
#include "ProcessUart/Provision.hpp"
#include "Mqtt/Mqtt.hpp"
#include "logging/slog.h"
#include "sqlite3.h"

using namespace std;

pthread_t vrpth_MqttThread;
pthread_t vrpth_UartReadThread;
pthread_t vrpth_UartWriteThread;

SLogConfig slgCfg;

int GetDBString(void *NotUsed, int argc, char **argv, char **azColName) {
	if(argc){
		int i,j;
		for(i = 0; i<argc; i++) {
			char *string = (char*)argv[i];
			for (j = 0; j < strlen(string); j++) {
				PRO_deviceKey[j] = string[j];
			}
			return 0;
		}
	}
	return 0;
}

/*
 * OpenDB(): open and query to db
 */
int OpenDB(char *sqlQuery, bool getString){
	//open DB
	//puts("Open db");
	sqlite3 *DB;
	char * msgError;
	int open = 0;
	do{
		open = sqlite3_open("/root/rd.Sqlite",&DB);
	}while(open != SQLITE_OK);

	//sqlite exec
	if(getString){
		while(sqlite3_exec(DB, sqlQuery, GetDBString, 0, &msgError) != 0){}
	}

	sqlite3_close(DB);
	return 0;
}

void GetDevicekey(){
	char *getDevKey = "SELECT DeviceKey FROM Device";
	OpenDB(getDevKey,true);
}

int main() {
	slog_init("logfile", SLOG_FLAGS_ALL, 1);
	slog_config_get(&slgCfg);
	slog_config_set(&slgCfg);
	GetDevicekey();
	pthread_create(&vrpth_MqttThread, NULL, MQTT_Thread, NULL);
	pthread_create(&vrpth_UartReadThread, NULL, GWINF_Thread, NULL);
	pthread_create(&vrpth_UartWriteThread, NULL, GWITF_WriteUart, NULL);

	pthread_join(vrpth_MqttThread,NULL);
	pthread_join(vrpth_UartReadThread,NULL);
	pthread_join(vrpth_UartWriteThread,NULL);
	return 0;
}

//============================================================================
// Name        : HC_UART_CPP.cpp
// Author      : anh
// Version     :
// Copyright   : Your copyright notice
// Description : Hello World in C++, Ansi-style
//============================================================================


#include "ProcessUart/GateInterface.hpp"
#include "Mqtt/Mqtt.hpp"
#include "logging/slog.h"

#define VERSION "1.2.11"

pthread_t vrpth_MqttThread;
pthread_t vrpth_UartReadThread;

SLogConfig slgCfg;


int main() {
	slog_init("logfile", SLOG_FLAGS_ALL, 1);
	slog_config_get(&slgCfg);
	slog_config_set(&slgCfg);
	slog_print(SLOG_INFO, 1, "version: %s", VERSION);

	pthread_create(&vrpth_MqttThread, NULL, MQTT_Thread, NULL);
	pthread_create(&vrpth_UartReadThread, NULL, GWINF_Thread, NULL);

	pthread_join(vrpth_MqttThread,NULL);
	pthread_join(vrpth_UartReadThread,NULL);
//	return 0;
}





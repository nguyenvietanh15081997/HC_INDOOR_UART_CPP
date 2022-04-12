/*
 * MQTT.c
 */

#include "../Mqtt/Mqtt.hpp"
#include "../JsonHandle/JsonProcess.hpp"
#include "../logging/slog.h"
#include "../Include/Include.hpp"
#include "../ProcessUart/RingBuffer.h"
#include <fstream>

#define MAXDATAMQTT			2048

struct mosquitto *mosq;
static ringbuffer_t vr_RingBufDataMqtt;
static char qos = 1;
static int run = 1;

using namespace std;
using namespace rapidjson;

//queue<bufferDataMqtt_t> bufferMqtt;

int mqtt_send(struct mosquitto *mosq, char *topic, char *msg) {
	mosquitto_publish(mosq, NULL, topic, strlen(msg), msg, qos, 0);
//	slog_info("<mqtt>send: %s", msg);
	return 0;
}

void connect_callback(struct mosquitto *mosq, void *obj, int result) {
	slog_info("<mqtt>%s: Connect callback, rc=%d", mqtt_host, result);
	mosquitto_subscribe(mosq, NULL, TP_SUB, qos);
	mqtt_send(mosq, TP_PUB, "{\"CMD\":\"CONNECTED\"}");
}

void message_callback(struct mosquitto *mosq, void *obj, const struct mosquitto_message *message) {
	int lengthMsg = strlen((char *)message->payload);
#if 1
	bufferDataMqtt_t dataPush = {{0}};
	memcpy(dataPush.dataMqtt,message->payload,lengthMsg+1);
	Document document;
	document.Parse(dataPush.dataMqtt);
//	slog_info("<mqtt>receive: %s", dataPush.dataMqtt);
	if(document.IsObject()){
		ring_push_head((ringbuffer_t *)&vr_RingBufDataMqtt,(void *)(&dataPush));
	}
	else {
		slog_print(SLOG_WARN,1,"<mqtt>message invalid");
	}
	if (vr_RingBufDataMqtt.count > 0) {
		bufferDataMqtt_t dataPop = {{0}};
		ring_pop_tail(&vr_RingBufDataMqtt,(void *) &dataPop);
		JsonHandle((char *)dataPop.dataMqtt);
	}
	Document objDelete;
	document.Swap(objDelete);
#endif
}
static char mqtt_password[64] = {0};
static void GetPassMqtt(char* nameFile){
	FILE *file;
	if ((file = fopen(nameFile,"r")) != NULL){
		fscanf(file,"%[^\n]",mqtt_password);
		fclose(file);
		slog_print(SLOG_INFO,1,"pass:%s",(char *)mqtt_password);
	}
}

static bool CheckFile (const std::string& name) {
    if (FILE *file = fopen(name.c_str(), "r")) {
        fclose(file);
        return true;
    } else {
        return false;
    }
}

void* MQTT_Thread(void *argv) {
	bool CheckMqtt = false;
	CheckMqtt = CheckFile("/etc/PassMqtt.txt");
	if(CheckMqtt){
		char * locationFile = "/etc/PassMqtt.txt";
		GetPassMqtt(locationFile);
	}

	slog_print(SLOG_INFO, 1, "Thread mqtt start");
	ring_init(&vr_RingBufDataMqtt,MAXDATAMQTT,sizeof(struct bufferDataMqtt));
	char clientid[24];
	int rc = 0;
	int abc = 0;

	mosquitto_lib_init();

	memset(clientid, 0, 24);
	snprintf(clientid, 23, "mysql_log_%d", getpid());
	mosq = mosquitto_new(clientid, true, 0);
	if (mosq) {
		mosquitto_connect_callback_set(mosq, connect_callback);
		mosquitto_message_callback_set(mosq, message_callback);

		if(CheckMqtt == false){
			cout << "check file MQTT: " << CheckMqtt << endl;
			rc = mosquitto_tls_set(mosq, "/etc/mosquitto/ca.crt", NULL, NULL, NULL, NULL);
			rc = mosquitto_tls_insecure_set(mosq, false);
			rc = mosquitto_connect(mosq, mqtt_host, mqtts_port, 60);
		}
		else{
			rc = mosquitto_username_pw_set(mosq, mqtt_username, (const char *)mqtt_password);
			rc = mosquitto_connect(mosq, mqtt_host, mqtt_port, 60);
		}
		mosquitto_subscribe(mosq, NULL, TP_SUB, qos);
		while (run) {
			rc = abc = mosquitto_loop(mosq, -1, 1);
			if (run && rc) {
				slog_warn("Connection mqtt error");
				sleep(4);
				mosquitto_reconnect_async(mosq);
			}
			usleep(1000);
		}
		mosquitto_destroy(mosq);
	}
	mosquitto_lib_cleanup();
	return NULL;
}


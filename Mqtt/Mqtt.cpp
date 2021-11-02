/*
 * MQTT.c
 */

#include "../Mqtt/Mqtt.hpp"
#include "../JsonHandle/JsonProcess.hpp"
#include "../logging/slog.h"
#include <time.h>

clock_t start, end;
struct mosquitto *mosq;

char qos = 1;
int run = 1;
bool countdown= false;
bool hasRsp = false;
bool isSetup = true;
uint8_t timeSendNode = 0;
bool delHead = false;
uint32_t timeout;

using namespace std;
using namespace rapidjson;
queue<char * > bufferMqtt;

int mqtt_send(struct mosquitto *mosq, char *topic, char *msg) {
	mosquitto_publish(mosq, NULL, topic, strlen(msg), msg, qos, 0);
	slog_info("<mqtt>send: %s", msg);
	return 0;
}

void connect_callback(struct mosquitto *mosq, void *obj, int result) {
	slog_info("<mqtt>%s: Connect callback, rc=%d", mqtt_host, result);
	mosquitto_subscribe(mosq, NULL, TP_SUB, qos);
	mqtt_send(mosq, TP_PUB, "{\"CMD\":\"CONNECTED\"}");
}

void message_callback(struct mosquitto *mosq, void *obj, const struct mosquitto_message *message) {
	char *msg = (char*) message->payload;
	Document document;
	document.Parse(msg);
	if(document.IsObject()){
		slog_info("<mqtt>receive: %s", msg);
		bufferMqtt.push(msg);
	}
	else {
		slog_print(SLOG_WARN,1,"<mqtt>message invalid");
	}
	if (bufferMqtt.size() > 0) {
		char *jobj = bufferMqtt.front();
		JsonHandle(jobj);
		bufferMqtt.pop();
	}
}

void* MQTT_Thread(void *argv) {
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

		abc = mosquitto_username_pw_set(mosq, mqtt_username, mqtt_password);
		rc = mosquitto_connect(mosq, mqtt_host, mqtt_port, 60);

		mosquitto_subscribe(mosq, NULL, TP_SUB, qos);
		while (run) {
			rc = abc = mosquitto_loop(mosq, -1, 1);
			if(bufferMqtt.size() == 0) {
				queue<char * > bufferEmpty;
				swap(bufferMqtt, bufferEmpty);
			}
			if (run && rc) {
				slog_warn("Connection mqtt error");
				sleep(4);
				mosquitto_reconnect_async(mosq);
			}
			usleep(10000);
		}
		mosquitto_destroy(mosq);
	}
	mosquitto_lib_cleanup();
	return NULL;
}


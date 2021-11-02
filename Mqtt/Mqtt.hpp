
#ifndef MQTT_HPP_
#define MQTT_HPP_

#include <mosquitto.h>
#include "../Include/Include.hpp"

#define mqtt_host 			"10.10.10.1"
#define mqtt_port 			1883
#define mqtt_username 		"RD"
#define mqtt_password 		"1"

#define TP_PUB           	"RD_STATUS"
#define TP_SUB				"RD_CONTROL"

extern struct mosquitto *mosq;
extern bool hasRsp;

/*
 * Transmit mqtt
 *
 * @param mosq mosquitto
 * @param topic topic mqtt
 * @param msg message mqtt
 * @return null
 */
int mqtt_send(struct mosquitto *mosq, char *topic, char *msg);

/*
 * Callback connect to mqtt broker
 *
 * @param mosq mosquitto
 * @param obj
 * @param result number of automatic reconnections
 * @return null
 */
void connect_callback(struct mosquitto *mosq, void *obj, int result);

/*
 * Receive and process mqtt
 * In this function call function handle message comming
 *
 * @param mosq mosquitto
 * @param obj
 * @param message message mqtt comming
 * @return null
 */
void message_callback(struct mosquitto *mosq, void *obj, const struct mosquitto_message *message);

/*
 * Thead manage mqtt
 * - keep connect to mqtt broker
 * - listen message coming
 */
void * MQTT_Thread(void *argv);

#endif

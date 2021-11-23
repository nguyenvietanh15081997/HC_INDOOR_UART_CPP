
#ifndef MQTT_HPP_
#define MQTT_HPP_

#include <mosquitto.h>

#define mqtt_host 			"10.10.10.1"
#define mqtt_port 			1883
#define mqtt_username 		"RD"

#define TP_PUB           	"RD_STATUS"
#define TP_SUB				"RD_CONTROL"

extern struct mosquitto *mosq;

/*
 * Thead manage mqtt
 * - keep connect to mqtt broker
 * - listen message coming
 */
void * MQTT_Thread(void *argv);

#endif

#pragma once

#include <functional>
#define MQTT_CALLBACK_SIGNATURE std::function<void(char*, uint8_t*, unsigned int)> callback
#define MQTT_LASTWILL_ONLINE                   "online"
#define MQTT_LASTWILL_OFFLINE                  "offline"
#define MQTT_LASTWILL_TOPIC                    "lastwill"


void network_init();
void network_shutdown();
void network_handle();
bool network_publishToMQTT(const char* topic, const char* payload, bool retained);
void network_mqtt_callback(MQTT_CALLBACK_SIGNATURE);
void network_mqtt_disconnect();
bool network_is_connected();
void network_flush();

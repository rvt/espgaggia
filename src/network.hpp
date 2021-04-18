#pragma once

#include <functional>
#define MQTT_CALLBACK_SIGNATURE std::function<void(char*, uint8_t*, unsigned int)> callback
#define MQTT_LASTWILL_ONLINE                   "online"
#define MQTT_LASTWILL_OFFLINE                  "offline"
#define MQTT_LASTWILL_TOPIC                    "lastwill"
#define OTA_CALLBACK_SIGNATURE std::function<void()>


void network_init();
void network_shutdown();
// Note that you must call this handle from the same task as you call esp_task_wdt_init().
void network_handle();
bool network_publishToMQTT(const char* topic, const char* payload, bool retained);
void network_mqtt_callback(MQTT_CALLBACK_SIGNATURE);
void network_mqtt_disconnect();
bool network_is_connected();
void network_flush();
void network_ota_begin_callback(OTA_CALLBACK_SIGNATURE callback);

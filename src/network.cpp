#include <cstring>
#include <functional>

#include "network.hpp"
#include <statemachine.hpp>
#include <propertyutils.hpp>

#include <PubSubClient.h>
#include <WiFi.h>
#include <esp_wifi.h>
#include <ESPmDNS.h>



/*********************
 *      EXTERNALS
 *********************/
extern Properties controllerConfig;

/*********************
 *      Variables
 *********************/

constexpr uint8_t LINE_BUFFER_SIZE = 128;

// Statemachine to handle (re)connection to MQTT
static State* BOOTSEQUENCESTART;
static State* DELAYEDMQTTCONNECTION;
static State* TESTMQTTCONNECTION;
static State* CONNECTMQTT;
static State* PUBLISHONLINE;
static State* SUBSCRIBECOMMANDTOPIC;
static State* WAITFORCOMMANDCAPTURE;
static StateMachine* bootSequence;;

static WiFiClient network_wifiClient;
static PubSubClient mqttClient(network_wifiClient);

static bool network_hasMqttConfigured = false;


/**
 * Setup statemachine that will handle reconnection to mqtt after WIFI drops
 */
void network_init() {
    // Needed for ESP32, otherwhise crash
    WiFi.mode(WIFI_STA); // explicitly set mode, esp defaults to STA+AP

    BOOTSEQUENCESTART = new State;
    DELAYEDMQTTCONNECTION = new StateTimed {1500};
    TESTMQTTCONNECTION = new State;
    CONNECTMQTT = new State;
    PUBLISHONLINE = new State;
    SUBSCRIBECOMMANDTOPIC = new State;
    WAITFORCOMMANDCAPTURE = new StateTimed { 3000 };

    BOOTSEQUENCESTART->setRunnable([]() {
        return TESTMQTTCONNECTION;
    });
    DELAYEDMQTTCONNECTION->setRunnable([]() {
        network_hasMqttConfigured =
            controllerConfig.contains("mqttServer") &&
            std::strlen((const char*)controllerConfig.get("mqttServer")) > 0;

        if (!network_hasMqttConfigured) {
            return DELAYEDMQTTCONNECTION;
        }

        return TESTMQTTCONNECTION;
    });
    TESTMQTTCONNECTION->setRunnable([]() {
        if (mqttClient.connected())  {
            if (WiFi.status() != WL_CONNECTED) {
                mqttClient.disconnect();
            }

            return DELAYEDMQTTCONNECTION;
        }

        // For some reason the access point active, so we disable it explicitly
        // FOR ESP32 we will keep on this state untill WIFI is connected
        if (WiFi.status() == WL_CONNECTED) {
            WiFi.mode(WIFI_STA);
        } else {
            return TESTMQTTCONNECTION;
        }

        return CONNECTMQTT;
    });
    CONNECTMQTT->setRunnable([]() {
        mqttClient.setServer(
            controllerConfig.get("mqttServer"),
            (int16_t)controllerConfig.get("mqttPort")
        );

        if (mqttClient.connect(
                controllerConfig.get("mqttClientID"),
                controllerConfig.get("mqttUsername"),
                controllerConfig.get("mqttPassword"),
                controllerConfig.get("mqttLastWillTopic"),
                0,
                1,
                MQTT_LASTWILL_OFFLINE)
           ) {
            return PUBLISHONLINE;
        }

        return DELAYEDMQTTCONNECTION;
    });
    PUBLISHONLINE->setRunnable([]() {
        network_publishToMQTT(
            MQTT_LASTWILL_TOPIC,
            MQTT_LASTWILL_ONLINE);
        return SUBSCRIBECOMMANDTOPIC;
    });
    SUBSCRIBECOMMANDTOPIC->setRunnable([]() {
        char mqttSubscriberTopic[32];
        strncpy(mqttSubscriberTopic, controllerConfig.get("mqttBaseTopic"), sizeof(mqttSubscriberTopic));
        strncat(mqttSubscriberTopic, "/+", sizeof(mqttSubscriberTopic));

        if (mqttClient.subscribe(mqttSubscriberTopic, 0)) {
            // Serial.println(mqttSubscriberTopic);
            return WAITFORCOMMANDCAPTURE;
        }

        mqttClient.disconnect();
        return DELAYEDMQTTCONNECTION;
    });
    WAITFORCOMMANDCAPTURE->setRunnable([]() {
        return TESTMQTTCONNECTION;
    });
    bootSequence = new StateMachine { BOOTSEQUENCESTART };
    bootSequence->start();
    MDNS.begin(controllerConfig.get("mqttClientID"));
    MDNS.addService("http", "tcp", 80);
}


/**
 * Publish a message to mqtt
 */
void network_publishToMQTT(const char* topic, const char* payload) {
    if (!mqttClient.connected()) {
        //  Serial.println(F("MQTT not connected"));
        return;
    }

    char buffer[LINE_BUFFER_SIZE];
    const char* mqttBaseTopic = controllerConfig.get("mqttBaseTopic");
    snprintf(buffer, sizeof(buffer), "%s/%s", mqttBaseTopic, topic);

    if (!mqttClient.publish(buffer, payload, true)) {
        Serial.print(F("Failed to publish : "));
        Serial.print(topic);
        Serial.print(F(" : "));
        Serial.print(payload);
    }
}

void network_mqtt_callback(MQTT_CALLBACK_SIGNATURE) {
    mqttClient.setCallback(callback);
}

void network_handle() {
    bootSequence->handle();
    mqttClient.loop();
}

void network_mqtt_disconnect() {
    mqttClient.disconnect();
}
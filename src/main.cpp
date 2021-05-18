/*
 */
#include <memory>
#include <cstring>
#include <vector>

#include "display.hpp"
#include <ui/gaggia_ui.h>
#include <gaggiascriptcontext.hpp>
#include "gaggiascripting.hpp"
#include "network.hpp"
#include <esp_task_wdt.h>
extern "C" {
#include <crc16.h>
}

#include <FS.h>
#include <SPIFFS.h>
#define FileSystemFS SPIFFS
#define FileSystemFSBegin() SPIFFS.begin(true)

#define WIFI_getChipId() (uint32_t)ESP.getEfuseMac()

#include "message.hpp"
#include <WiFiManager.h> // https://github.com/tzapu/WiFiManager
#include <propertyutils.hpp>
#include <optparser.hpp>
#include <utils.h>
#include <onoffheatelement.hpp>
#include <oneshot.hpp>
#include <ui/gaggia_ui.h>

#include <SPI.h> // Include for harware SPI
#include <Wire.h>

#include <config.hpp>
#include <Fuzzy.h>

#include <gaggiaClassicController.hpp>
#include <gaggiaClassicControllerPID.hpp>
#include <boiler.hpp>
#include <StreamUtils.h>
#include <gaggiaio.hpp>
#include <gaggiahwio.hpp>
typedef PropertyValue PV;

// of transitions
uint32_t counter50TimesSec = 1;

// Number calls per second we will be handling
constexpr uint16_t TICK50MS_PERIOD = (1000 / 50);
constexpr uint16_t TICK10MS_PERIOD = (1000 / 100);
constexpr uint8_t LINE_BUFFER_SIZE = 128;
constexpr uint8_t PARAMETER_SIZE = 16;

// WiFI Manager
WiFiManager wifiManager;
#define MQTT_SERVER_LENGTH 40
#define MQTT_PORT_LENGTH 5
#define MQTT_USERNAME_LENGTH 18
#define MQTT_PASSWORD_LENGTH 18
WiFiManagerParameter wifiManager_mqtt_server("server", "mqtt server", "", MQTT_SERVER_LENGTH);
WiFiManagerParameter wifiManager_mqtt_port("port", "mqtt port", "", MQTT_PORT_LENGTH);
WiFiManagerParameter wifiManager_mqtt_user("user", "mqtt username", "", MQTT_USERNAME_LENGTH);

const char _customHtml_hidden[] = "type=\"password\"";
WiFiManagerParameter wifiManager_mqtt_password("input", "mqtt password", "", MQTT_PASSWORD_LENGTH, _customHtml_hidden, WFM_LABEL_AFTER);


// Stores information about the controller on communication level
Properties controllerConfig;
volatile bool controllerConfigModified = false;
// Stores information about the current temperature settings
Properties gaggiaConfig;
volatile bool gaggiaConfigModified = false;

// CRC value of last update to MQTT
uint16_t lastMeasurementCRC = 0;
// Indicate that a service requested an restart. Set to millies() of current time and it will restart 5000ms later
uint32_t shouldRestart = 0;

// I/O Sensors and devices
std::unique_ptr<Boiler> controller(nullptr);
GaggiaHWIO gaggiaIO {
    PERI_PIN_SPI_CLK,
    PERI_PIN_SPI_MISO,
    BREW_PIN_SPI_CS,
    STEAM_PIN_SPI_CS,
    BREW_BUTTON_PIN,
    STEAM_BUTTON_PIN,
    BOILER_PIN,
    PUMP_PIN,
    VALVE_PIN
};

#if defined (GUI_IO)
bool uiBrewButton = false;
bool uiSteamButton = false;
#endif

constexpr uint8_t MAX_MENU_ENTRIES = 10;
static char gaggia_menu_map[MAX_MENU_STR_LENGTH];
static const char* gaggia_menu_ptr_map[10];
static char const* LAST_MENU_ENTRY = "";

QueueHandle_t xMainMessageQueue;
QueueHandle_t xUIMessageQueue;

typedef MainQueue_message<MainMessage_e, 16> MainMessage_t;
typedef UIQueue_message<UIMessage_e, 32> UIMessage_t;


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////
bool saveConfig(const char* filename, Properties& properties);

OneShot saveGaggiaConfigHandler{
    5000,
    []() {
    },
    []() {
        saveConfig(CONFIG_FILENAME, gaggiaConfig);
        gaggiaConfigModified = false;
        saveGaggiaConfigHandler.start();
    },
    []() {
        return gaggiaConfigModified;
    }
};

OneShot saveHardwareConfigHandler{
    5000,
    []() {
    },
    []() {
        saveConfig(CONTROLLER_CONFIG_FILENAME, controllerConfig);
        controllerConfigModified = false;
        saveHardwareConfigHandler.start();
    },
    []() {
        return controllerConfigModified;
    }
};

OneShot removeCounterLabel{
    5000,
    []() {
        UIMessage_t setVisibilityMessage {UIMessage_e::SET_VISIBILITY, TIMER_BOX, (bool)true};
        xQueueSend(xUIMessageQueue, (void*) &setVisibilityMessage, (TickType_t) 1);
    },
    []() {
        removeCounterLabel.start();
        // UIMessage_t setVisibilityMessage {UIMessage_e::SET_VISIBILITY};
        // UIVisibility_t data{TIMER_BOX, false};
        // setVisibilityMessage.setData(&data, sizeof(UIVisibility_t));
        // xQueueSend( xUIMessageQueue, ( void * ) &setVisibilityMessage, ( TickType_t ) 10 );
        UIMessage_t setVisibilityMessage {UIMessage_e::SET_VISIBILITY, TIMER_BOX, (bool)false};
        xQueueSend(xUIMessageQueue, (void*) &setVisibilityMessage, (TickType_t) 1);
    },
    []() {
        if (gaggiaIO.pump()) {
            removeCounterLabel.hold();
        }

        return gaggiaIO.pump();
    }
};

OneShot powerDownMonitor {
    //TODO: Make this a configuration
    60 * 1000 * 45,
    []() {},
    []() {
        gaggia_scripting_load(POWERDOWN_SCRIPT);
    },
    []() {
        return false;
    }
};

OneShot powerSaveMonitor {
    //TODO: Make this a configuration
    60 * 1000 * 15,
    []() {
    },
    []() {
        gaggia_scripting_load(POWERSAVE_SCRIPT);
    },
    []() {
        return false;
    }
};

void updateUI();
OneShot uiUpdateTimer {
    100,
    []() {
    },
    []() {
        updateUI();
        uiUpdateTimer.start();
    },
    []() {
        return true;
    }
};

bool loadConfig(const char* filename, Properties& properties) {
    bool ret = false;

    if (FileSystemFSBegin()) {
        if (FileSystemFS.exists(filename)) {
            //file exists, reading and loading
            Serial.print(F("Opening : "));
            Serial.print(filename);
            File configFile = FileSystemFS.open(filename, "r");

            if (configFile) {
                Serial.println(F(" loaded."));
                deserializeProperties<LINE_BUFFER_SIZE>(configFile, properties);
                serializeProperties<LINE_BUFFER_SIZE>(Serial, properties);
                configFile.close();
            }

        } else {
            Serial.print(F(" failed."));
            Serial.println(filename);
        }

        // FileSystemFS.end();
    } else {
        Serial.print(F("Failed to begin FileSystemFS"));
    }

    return ret;
}


/**
 * Store custom parameter configuration in FileSystemFS
 */
bool saveConfig(const char* filename, Properties& properties) {
    bool ret = false;

    if (FileSystemFSBegin()) {
        FileSystemFS.remove(filename);
        Serial.print(F("Opening : "));
        Serial.print(filename);
        File configFile = FileSystemFS.open(filename, "w");

        if (configFile) {
            Serial.println(F(" Saved."));
            serializeProperties<LINE_BUFFER_SIZE>(configFile, properties);
            serializeProperties<LINE_BUFFER_SIZE>(Serial, properties, false);
            ret = true;
            configFile.close();
        } else {
            Serial.print(F(" Failed."));
        }

    }

    return ret;
}

///////////////////////////////////////////////////////////////////////////
//  Automation
///////////////////////////////////////////////////////////////////////////
void stopCurrentScript() {
    xQueueReset(xMainMessageQueue);
    MainMessage_t loadScriptMessage {MainMessage_e::LOAD_SCRIPT};
    loadScriptMessage.copyChar(STOP_SCRIPT);
    xQueueSend(xMainMessageQueue, (void*) &loadScriptMessage, pdMS_TO_TICKS(1));
}

void handleScriptContext() {

#if defined (GUI_IO)


#if defined (GUI_BUTTONS)
    gaggia_ui_set_led(BREW_BUT_STATUS, uiBrewButton);
    gaggia_ui_set_led(STEAM_BUT_STATUS, uiSteamButton);
#endif
    gaggia_ui_set_led_bright(HEAT_STATUS_SSR, gaggiaIO.heatElement()->power() * 2);

    if (gaggia_scripting_context() != nullptr) {
        gaggia_ui_set_led(VALVE_STATUS_SSR, gaggia_scripting_context()->m_valve);
        gaggia_ui_set_led(PUMP_STATUS_SSR, gaggia_scripting_context()->m_pump);
    }

#endif

    if (gaggia_scripting_context() != nullptr) {
        gaggia_scripting_context()->m_brewButton = gaggiaIO.brewButton()->current();
        gaggia_scripting_context()->m_steamButton = gaggiaIO.steamButton()->current();
        gaggia_scripting_context()->m_steamTemperature = gaggiaIO.steamTemperature()->get();
        gaggia_scripting_context()->m_brewTemperature = gaggiaIO.brewTemperature()->get();
    }

    int8_t handle = gaggia_scripting_handle();

    switch (handle) {
        case 0:
            Serial.println("Script ended");
            gaggiaIO.pump(false);
            gaggiaIO.valve(false);
            controller->brewMode(true);
            controller->setPoint(gaggiaConfig.get("defaultBrewTemp"));
            gaggia_scripting_load(STARTUP_SCRIPT);
            break;

        case 2:
            gaggiaIO.resetStop();

        case 1:
            gaggiaIO.pump(gaggia_scripting_context()->m_pump);
            gaggiaIO.valve(gaggia_scripting_context()->m_valve);
            controller->brewMode(gaggia_scripting_context()->m_brewMode);
            controller->setPoint(gaggia_scripting_context()->m_setPoint);
            break;
    }

#if defined (GUI_BUTTONS)
    uiBrewButton = gaggiaIO.brewButton()->current();
    uiSteamButton = gaggiaIO.steamButton()->current();
#endif

    if (gaggia_scripting_context()!=nullptr) {
        if (gaggia_scripting_context()->m_monitorButtonStop) {
            if (gaggiaIO.stop()) {
                stopCurrentScript();
            }
        } else {
            gaggiaIO.resetStop();
        }
    }
}


float round05(float input) {
    return (floor((input * 2) + 0.5f) / 2.0f);
}
///////////////////////////////////////////////////////////////////////////
//  MQTT
///////////////////////////////////////////////////////////////////////////
/*
* Publish current status
* to = temperature oven
* sp = set Point
* f1 = Speed of fan 1
* lo = Lid open alert
* lc = Low charcoal alert
* ft = Type of ventilator
*/
void publishStatusToMqtt() {
    char* format;
    char* buffer;

    // Can we do better than this?
    constexpr uint8_t numitems = 8;

    if (controllerConfig.get("statusJson")) {
        static char f[] = "{\"tempBrew\":%.2f,\"tempSteam\":%.2f,\"setPoint\":%.2f,\"boiler\":%.2f,\"brewBut\":%d,\"steamBut\":%d,\"pump\":%d,\"valve\":%d}";
        static char b[sizeof(f) + numitems * 3]; // 3 bytes per extra item + 10 extra
        format = f;
        buffer = b;
    } else {
        static char f[] = "tempBrew=%.2f tempSteam=%.2f setPoint=%.2f boiler=%.2f brewBut=%d steamBut=%d pump=%d valve=%d";
        static char b[sizeof(f) + numitems * 3]; // 3 bytes per extra item + 10 extra
        format = f;
        buffer = b;
    }

    sprintf(buffer,
            format,
            round05(gaggiaIO.brewTemperature()->get()),
            round05(gaggiaIO.steamTemperature()->get()),
            round05(controller->setPoint()),
            round05(gaggiaIO.heatElement()->power()),
            gaggiaIO.brewButton()->current(),
            gaggiaIO.steamButton()->current(),
            gaggiaIO.pump(),
            gaggiaIO.valve()
           );


    // Quick hack to only update when data actually changed
    uint16_t thisCrc = crc16((uint8_t*)buffer, std::strlen(buffer));

    if (thisCrc != lastMeasurementCRC) {
        if (network_publishToMQTT("status", buffer, true)) {
            lastMeasurementCRC = thisCrc;
        }
    }
}

/**
 * Handle incomming MQTT requests
 */
void handleCmd(const char* topic, const char* p_payload) {
    auto topicPos = topic + strlen(controllerConfig.get("mqttBaseTopic"));
    // Serial.print(F("Handle command : "));
    // Serial.print(topicPos);
    // Serial.print(F(" : "));
    // Serial.println(p_payload);

    // Look for a temperature setPoint topic
    char payloadBuffer[LINE_BUFFER_SIZE];
    strncpy(payloadBuffer, p_payload, sizeof(payloadBuffer));

    if (std::strstr(topicPos, "/controllerConfig") != nullptr) {
        Serial.println(F("controllerConfig received"));
        StringStream stream;
        stream.print(payloadBuffer);
        deserializeProperties<LINE_BUFFER_SIZE>(stream, controllerConfig);
        controllerConfigModified = true;
    }

    // List all files in base directory over Serial
    if (strstr(topicPos, "/listFiles") != nullptr) {
        File root = FileSystemFS.open("/");
        File file = root.openNextFile();

        while (file) {
            Serial.print("FILE: ");
            Serial.println(file.name());
            file = root.openNextFile();
        }
    }

    if (strstr(topicPos, "/reset") != nullptr) {
        if (strcmp(payloadBuffer, "1") == 0) {
            shouldRestart = millis();
        }
    }

    if (strstr(topicPos, "/config") != nullptr) {
        OptParser::get(payloadBuffer, [&](OptValue v) {
            if (strcmp(v.key(), "on") == 0) {
                if ((bool)v) {
                    gaggia_scripting_load(STARTUP_SCRIPT);
                    powerSaveMonitor.reset();
                    powerDownMonitor.reset();
                } else {
                    gaggia_scripting_load(POWERDOWN_SCRIPT);
                    powerSaveMonitor.stop();
                    powerDownMonitor.stop();
                }
            }
        });
    }
}

/**
 * Initialise MQTT and variables
 */
void setupMQTTCallback() {

    network_mqtt_callback([](char* p_topic, byte * p_payload, uint16_t p_length) {
        char mqttReceiveBuffer[LINE_BUFFER_SIZE];
        // Serial.println(p_topic);

        if (p_length >= sizeof(mqttReceiveBuffer)) {
            return;
        }

        memcpy(mqttReceiveBuffer, p_payload, p_length);
        mqttReceiveBuffer[p_length] = 0;
        handleCmd(p_topic, mqttReceiveBuffer);
    });

}

void turnOffHardware() {
    // Turn off boiler, pump and valve
    pinMode(BOILER_PIN, OUTPUT);
    pinMode(PUMP_PIN, OUTPUT);
    pinMode(VALVE_PIN, OUTPUT);
    digitalWrite(BOILER_PIN, true);
    digitalWrite(PUMP_PIN, true);
    digitalWrite(VALVE_PIN, true);
}

void setupNetworkCallback() {
    network_ota_begin_callback([]() {
        turnOffHardware();
    });
}

///////////////////////////////////////////////////////////////////////////
//  IOHardware
///////////////////////////////////////////////////////////////////////////
void setupIOHardware() {
    const char* controllerType = (const char*)gaggiaConfig.get("controller");;

    if (strcmp(controllerType, "PID") == 0) {
        auto gController = new GaggiaClassicControllerPID(&gaggiaIO);
        controller.reset(gController);
    } else {
        auto gController = new GaggiaClassicController(&gaggiaIO);
        controller.reset(gController);
        gController->init();
    }



    controller->setPoint(gaggiaConfig.get("defaultBrewTemp"));
    controller->brewMode(true);
}

///////////////////////////////////////////////////////////////////////////
//  Webserver/WIFIManager
///////////////////////////////////////////////////////////////////////////
void saveParamCallback() {
    Serial.println("[CALLBACK] saveParamCallback fired");

    if (std::strlen(wifiManager_mqtt_server.getValue()) > 0) {
        controllerConfig.put("mqttServer", PV(wifiManager_mqtt_server.getValue()));
        controllerConfig.put("mqttPort", PV(std::atoi(wifiManager_mqtt_port.getValue())));
        controllerConfig.put("mqttUsername", PV(wifiManager_mqtt_user.getValue()));
        controllerConfig.put("mqttPassword", PV(wifiManager_mqtt_password.getValue()));
        controllerConfigModified = true;
        // Redirect from MQTT so on the next reconnect we pickup new values
        network_mqtt_disconnect();
        // Send redirect back to param page
        wifiManager.server->sendHeader(F("Location"), F("/param?"), true);
        wifiManager.server->send(302, FPSTR(HTTP_HEAD_CT2), "");   // Empty content inhibits Content-length header so we have to close the socket ourselves.
        wifiManager.server->client().stop();
    }
}

/**
 * Setup the wifimanager and configuration page
 */
void setupWifiManager() {
    char port[6];
    snprintf(port, sizeof(port), "%d", (int16_t)controllerConfig.get("mqttPort"));
    wifiManager_mqtt_port.setValue(port, MQTT_PORT_LENGTH);
    wifiManager_mqtt_password.setValue(controllerConfig.get("mqttPassword"), MQTT_PASSWORD_LENGTH);
    wifiManager_mqtt_user.setValue(controllerConfig.get("mqttUsername"), MQTT_USERNAME_LENGTH);
    wifiManager_mqtt_server.setValue(controllerConfig.get("mqttServer"), MQTT_SERVER_LENGTH);

    wifiManager.addParameter(&wifiManager_mqtt_server);
    wifiManager.addParameter(&wifiManager_mqtt_port);
    wifiManager.addParameter(&wifiManager_mqtt_user);
    wifiManager.addParameter(&wifiManager_mqtt_password);

    /////////////////
    // set country
    wifiManager.setClass("invert");
    wifiManager.setCountry("US"); // setting wifi country seems to improve OSX soft ap connectivity, may help others as well

    // Set configuration portal
    wifiManager.setShowStaticFields(false);
    wifiManager.setConfigPortalBlocking(false); // Must be blocking or else AP stays active
    wifiManager.setDebugOutput(true);
    wifiManager.setSaveParamsCallback(saveParamCallback);
    wifiManager.setHostname(controllerConfig.get("mqttClientID"));
    std::vector<const char*> menu = {"wifi", "wifinoscan", "info", "param", "sep", "erase", "restart"};
    wifiManager.setMenu(menu);

    wifiManager.startWebPortal();
    wifiManager.autoConnect(controllerConfig.get("mqttClientID"));
}

///////////////////////////////////////////////////////////////////////////
//  UI Event handling
///////////////////////////////////////////////////////////////////////////

/**
 * Setup all events comming from the UI
 * All callbacks are savegarded by a mutex created in displayTask(...)
 */
void setup_ui_events() {
#if defined (GUI_IO)

#if defined (GUI_BUTTONS)
    gaggia_ui_add_event_cb(STEAM_TEMP_OBJ, [](enum ui_element_types label, enum ui_event event) {
        uiSteamButton = !uiSteamButton;
    });
    gaggia_ui_add_event_cb(BREW_TEMP_OBJ, [](enum ui_element_types label, enum ui_event event) {
        uiBrewButton = !uiBrewButton;
    });
#endif
#endif
    // Spinners
    gaggia_ui_spin_set_range(BREWTEMP_SPIN, BREW_TEMP_MIN, BREW_TEMP_MAX);
    gaggia_ui_spin_set_range(STEAMTEMP_SPIN, STEAM_TEMP_MIN, STEAM_TEMP_MAX);
    gaggia_ui_spin_set_value(BREWTEMP_SPIN, (float)gaggiaConfig.get("defaultBrewTemp"));
    gaggia_ui_spin_set_value(STEAMTEMP_SPIN, (float)gaggiaConfig.get("defaultSteamTemp"));

    //
    gaggia_ui_set_text_hint(TIMER_LABEL, "0.0", 6);
    gaggia_ui_set_text_hint(BREW_TEMP_LABEL, "0.0", 8);
    gaggia_ui_set_text_hint(STEAM_TEMP_LABEL, "0.0", 8);

    // A bit ugly, but it works... I guess...
    // Makes a map of PTR to each menu entry
    uint16_t pos = 0, menuItem = 0;
    const char* menuConfig = (const char*)gaggiaConfig.get("menuConfig");
    strncpy(gaggia_menu_map, menuConfig, sizeof(gaggia_menu_map));
    gaggia_menu_ptr_map[menuItem] = &gaggia_menu_map[pos];

    while (gaggia_menu_map[pos] != '\0') {
        if (gaggia_menu_map[pos] == ',') {
            if (menuItem < (MAX_MENU_ENTRIES - 2)) {
                menuItem++;
                gaggia_menu_ptr_map[menuItem] = &gaggia_menu_map[pos + 1];
            }

            gaggia_menu_map[pos] = '\0';
        } else if (gaggia_menu_map[pos] == '^') {
            gaggia_menu_map[pos] = '\n';
        }

        pos++;
    }

    gaggia_menu_ptr_map[++menuItem] = LAST_MENU_ENTRY;
    gaggia_ui_btn_map(PROCESS_SELECT_MATRIX, gaggia_menu_ptr_map);

    // Events
    gaggia_ui_add_event_cb(STOP_BUTTON, [](enum ui_element_types label, enum ui_event event) {
        stopCurrentScript();
    });

    gaggia_ui_add_event_cb(GENERIC_UI_INTERACTION, [](enum ui_element_types label, enum ui_event event) {
        const MainMessage_t messageSave {MainMessage_e::POWERSAVE_RESTART};
        xQueueSend(xMainMessageQueue, (void*) &messageSave, (TickType_t) 1);
    });

    gaggia_ui_add_event_cb(BREWTEMP_SPIN, [](enum ui_element_types label, enum ui_event event) {
        const MainMessage_t message {MainMessage_e::SET_DEFAULTBREWTEMPERATURE, gaggia_ui_spin_get_value(BREWTEMP_SPIN) * 1.0f};
        xQueueSend(xMainMessageQueue, (void*) &message, (TickType_t) 1);
    });

    gaggia_ui_add_event_cb(STEAMTEMP_SPIN, [](enum ui_element_types label, enum ui_event event) {
        const MainMessage_t message {MainMessage_e::SET_DEFAULTSTEAMTEMPERATURE, gaggia_ui_spin_get_value(STEAMTEMP_SPIN) * 1.0f};
        xQueueSend(xMainMessageQueue, (void*) &message, (TickType_t) 1);
    });

    gaggia_ui_add_event_cb(PROCESS_SELECT_MATRIX, [](enum ui_element_types label, enum ui_event event) {
        char filename[16];
        snprintf(filename, sizeof(filename), "/menu_%d.txt", gaggia_ui_btn_map_active(label));

        MainMessage_t loadScriptMessage {MainMessage_e::LOAD_SCRIPT};
        loadScriptMessage.copyChar(filename);
        xQueueSend(xMainMessageQueue, (void*) &loadScriptMessage, (TickType_t) 1);

        const UIMessage_t changeViewMessage {UIMessage_e::CHANGE_VIEW, _LAST_ITEM_STUB, (uint32_t)0};
        xQueueSend(xUIMessageQueue, (void*) &changeViewMessage, (TickType_t) 1);

    });
}

void updateUI() {
    auto currentMillis = millis();
    // Temporary untill we have a better spot
    char buffer[16];
    dtostrf(gaggiaIO.brewTemperature()->get(), 0, 0, buffer);
    UIMessage_t brewMessage{UIMessage_e::SET_TEXT_STATIC, BREW_TEMP_LABEL, buffer};
    xQueueSend(xUIMessageQueue, (void*) &brewMessage, (TickType_t) 1);

    dtostrf(gaggiaIO.steamTemperature()->get(), 0, 0, buffer);
    UIMessage_t steamMessage{UIMessage_e::SET_TEXT_STATIC, STEAM_TEMP_LABEL, buffer};
    xQueueSend(xUIMessageQueue, (void*) &steamMessage, (TickType_t) 1);

    removeCounterLabel.handle(currentMillis);

    if (gaggiaIO.pump()) {
        powerSaveMonitor.hold();
        powerDownMonitor.hold();
    }

    // Quick hack to ensure that we will always show the correct time on the display
    // If we just look at when the pump goes off, we  don't show correct timings
    static uint32_t last_pumpMillis = 1; // pumpMillis returns 1 after initialise
    const uint32_t pumpMillis = gaggiaIO.pumpMillis();

    if (last_pumpMillis != pumpMillis) {
        last_pumpMillis = pumpMillis;

        if (pumpMillis < 999000) {
            dtostrf(pumpMillis / 1000.f, 1, 1, buffer);
            UIMessage_t timerMessage{UIMessage_e::SET_TEXT_STATIC, TIMER_LABEL, (const char*)buffer};
            xQueueSend(xUIMessageQueue, (void*) &timerMessage, (TickType_t) 1);
        }
    }
}

///////////////////////////////////////////////////////////////////////////
//  SETUP and LOOP
///////////////////////////////////////////////////////////////////////////

void setDefaultConfigurations() {
    // controllerConfig
    char chipHexBuffer[9];
    snprintf(chipHexBuffer, sizeof(chipHexBuffer), "%08X", WIFI_getChipId());

    char mqttClientID[16];
    snprintf(mqttClientID, sizeof(mqttClientID), "gaggia_%s", chipHexBuffer);

    char mqttBaseTopic[16];
    snprintf(mqttBaseTopic, sizeof(mqttBaseTopic), "gaggia/%s", chipHexBuffer);

    char mqttLastWillTopic[64];
    snprintf(mqttLastWillTopic, sizeof(mqttLastWillTopic), "%s/%s", mqttBaseTopic, MQTT_LASTWILL_TOPIC);

    controllerConfigModified |= controllerConfig.putNotContains("mqttServer", PV(""));
    controllerConfigModified |= controllerConfig.putNotContains("mqttUsername", PV(""));
    controllerConfigModified |= controllerConfig.putNotContains("mqttPassword", PV(""));
    controllerConfigModified |= controllerConfig.putNotContains("mqttPort", PV(1883));
    controllerConfigModified |= controllerConfig.putNotContains("pauseForOTA", PV(true));

    controllerConfig.put("mqttClientID", PV(mqttClientID));
    controllerConfig.put("mqttBaseTopic", PV(mqttBaseTopic));
    controllerConfig.put("mqttLastWillTopic", PV(mqttLastWillTopic));
    //    controllerConfig.put("standbyTime", PV(15*60));

    // gaggiaConfig
    gaggiaConfigModified |= gaggiaConfig.putNotContains("defaultBrewTemp", PV(97.0f));
    gaggiaConfigModified |= gaggiaConfig.putNotContains("defaultSteamTemp", PV(145.0f));
    gaggiaConfigModified |= gaggiaConfig.putNotContains("powerSaveTemp", PV(50.0f));
    gaggiaConfigModified |= gaggiaConfig.putNotContains("standByTemp", PV(95.0f));
    gaggiaConfigModified |= gaggiaConfig.putNotContains("controller", PV("FUZZY"));
}


void handleUIMessageQueue() {
    UIMessage_t uiMessage_t{UIMessage_e::NOOP};

    uint8_t size = uxQueueMessagesWaiting(xUIMessageQueue);

    if (size > 6) {
        Serial.println("Large xUIMessageQueue");
    }

    // Ensure that we send with xUIMessageQueue and a ticktype of 1, when setting it to 0
    // it seems that xQueueReceive was blocking
    while (xQueueReceive(xUIMessageQueue, &(uiMessage_t), (TickType_t) 1) == pdPASS) {
        //Serial.print ((uint8_t)uiMessage_t.type);
        switch (uiMessage_t.type) {
            case UIMessage_e::CHANGE_VIEW:
                gaggia_ui_change_screen(uiMessage_t.intValue);
                break;

            case UIMessage_e::SET_VISIBILITY:
                gaggia_ui_set_visibility(uiMessage_t.element, uiMessage_t.boolValue);
                break;

            case UIMessage_e::SET_TEXT:
                gaggia_ui_set_text(uiMessage_t.element, uiMessage_t.charValue);
                break;

            case UIMessage_e::SET_TEXT_STATIC:
                char* buffer = gaggia_ui_set_text_buffer(uiMessage_t.element);
                strcpy(buffer, uiMessage_t.charValue);
                gaggia_ui_set_text(uiMessage_t.element, nullptr);
                break;
        }
    }
}

void _displayTask() {
    handleUIMessageQueue();
    //uint32_t sta = millis();
    display_loop();
    //uint32_t t = millis() - sta;

    //if (t > 100) {
    //    Serial.println(millis() - sta);
    //}
}

void displayTask(void* pvParameters) {
    esp_task_wdt_add(NULL);
    while (true) {
        esp_task_wdt_reset();
        _displayTask();
    }
}

void handleMainMessageQueue() {
    MainMessage_t mainMessage_t{MainMessage_e::NOOP};

    uint8_t size = uxQueueMessagesWaiting(xMainMessageQueue);

    if (size > 8) {
        Serial.println("Large xMainMessageQueue");
    }

    if (xQueueReceive(xMainMessageQueue, &(mainMessage_t), (TickType_t) 1) == pdPASS) {
        switch (mainMessage_t.type) {
            case MainMessage_e::POWERSAVE_RESTART:
                powerSaveMonitor.start();
                powerSaveMonitor.trigger();
                powerDownMonitor.start();
                powerDownMonitor.trigger();
                break;

            case MainMessage_e::SET_DEFAULTBREWTEMPERATURE:
                gaggiaConfig.put("defaultBrewTemp", PV(mainMessage_t.floatValue));
                gaggiaConfigModified = true;
                break;

            case MainMessage_e::SET_DEFAULTSTEAMTEMPERATURE:
                gaggiaConfig.put("defaultSteamTemp", PV(mainMessage_t.floatValue));
                gaggiaConfigModified = true;
                break;

            case MainMessage_e::LOAD_SCRIPT:
                gaggia_scripting_load(mainMessage_t.charValue);
                break;

            default:
                break;
        }
    }
}

// Keep track when the last time we ran the effect state changes
uint32_t tick50Millis = 0;
uint32_t tick10Millis = 0;
uint8_t maxSlots = 255;
void loop() {
    const uint32_t currentMillis = millis();

    if (currentMillis - tick50Millis >= TICK50MS_PERIOD) {

#if defined(DISPLAY_TASK_IN_MAIN_LOOP)
        _displayTask();
#endif
        handleMainMessageQueue();

        tick50Millis = currentMillis;
        counter50TimesSec++;

        controller -> handle(currentMillis);

        // once a second publish status to mqtt (if there are changes)
        if (counter50TimesSec % 25 == 0) {
            publishStatusToMqtt();
        }

        // Maintenance stuff
#if defined(SHOW_FREE_HEAP)

        if (counter50TimesSec % 50 == 0) {
            Serial.println(ESP.getFreeHeap());
        }

#endif

        uint8_t slot50 = 0;

        if (counter50TimesSec % maxSlots == slot50++) {
            network_handle();
        } else if (counter50TimesSec % maxSlots == slot50++) {
            saveHardwareConfigHandler.handle(currentMillis);
        } else if (counter50TimesSec % maxSlots == slot50++) {
            saveGaggiaConfigHandler.handle(currentMillis);
        } else if (counter50TimesSec % maxSlots == slot50++) {
            powerSaveMonitor.handle(currentMillis);
            powerDownMonitor.handle(currentMillis);
            uiUpdateTimer.handle(currentMillis);
        } else if (counter50TimesSec % maxSlots == slot50++) {
            esp_task_wdt_reset();
        } else if (counter50TimesSec % maxSlots == slot50++) {
            wifiManager.process();
        } else if (counter50TimesSec % maxSlots == slot50++) {
            if (shouldRestart != 0 && (currentMillis - shouldRestart >= 5000)) {
                shouldRestart = 0;
                ESP.restart();
            }
        } else {
            maxSlots = slot50;
        }
    }

    // Gaggia IO has it's own timer
    gaggiaIO.handle(currentMillis);

    if (currentMillis - tick10Millis >= TICK10MS_PERIOD) {
        tick10Millis += TICK10MS_PERIOD;
        handleScriptContext();
    }

}


void setup() {
    /* Pins'a go low quickly, but we need them to be high so the SSR's dont turn on.
    So we do not want to wait for delays and other startup items */
    turnOffHardware();

    pinMode(CONFIG_LV_DISP_SPI_CS, OUTPUT);    // No CSS for display, so we keep it low all the time (saves some cycles)
    digitalWrite(CONFIG_LV_DISP_SPI_CS, LOW);

    // Enable serial port
    Serial.begin(115200);
    // Wait until Serial comes available
    uint8_t counter = 0;

    while (!Serial && counter < 10) {
        delay(5 * counter++);
    }

    xMainMessageQueue = xQueueCreate(10, sizeof(MainMessage_t));
    xUIMessageQueue = xQueueCreate(8, sizeof(UIMessage_t));

    if (xMainMessageQueue != nullptr) {
        Serial.println("xMainMessageQueue created");
    }

    if (xUIMessageQueue != nullptr) {
        Serial.println("xUIMessageQueue created");
    }


    // load configurations
    loadConfig(CONTROLLER_CONFIG_FILENAME, controllerConfig);
    loadConfig(CONFIG_FILENAME, gaggiaConfig);
    setDefaultConfigurations();

    setupIOHardware();
    gaggia_scripting_init(&gaggiaIO);
    gaggia_scripting_load(STARTUP_SCRIPT);

    network_init();
    setupMQTTCallback();
    setupNetworkCallback();
    setupWifiManager();
    display_init();
    setup_ui_events();

    powerSaveMonitor.trigger();
    powerDownMonitor.trigger();
    uiUpdateTimer.trigger();

#if ! defined(DISPLAY_TASK_IN_MAIN_LOOP)
    xTaskCreatePinnedToCore(
        displayTask,
        "displayUpdateTask",
        10000,      /* Stack size in words */
        NULL,
        0,
        NULL,
        0);         /* Core ID */
#endif

    esp_task_wdt_init(WDT_TIMEOUT, true);
    esp_task_wdt_add(NULL);

    tick10Millis = millis();
    tick50Millis = millis();
}
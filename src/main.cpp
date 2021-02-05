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
extern "C" {
#include <crc16.h>
}

#include <FS.h>
#include <SPIFFS.h>
#define FileSystemFS SPIFFS
#define FileSystemFSBegin() SPIFFS.begin(true)

#define WIFI_getChipId() (uint32_t)ESP.getEfuseMac()


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
#include <boiler.hpp>
#include <StreamUtils.h>
#include <gaggiaio.hpp>
#include <gaggiahwio.hpp>
typedef PropertyValue PV;

// of transitions
uint32_t counter50TimesSec = 1;

// Number calls per second we will be handling
constexpr uint8_t FRAMES_PER_SECOND        = 50;
constexpr uint16_t EFFECT_PERIOD_CALLBACK = (1000 / FRAMES_PER_SECOND);
constexpr uint8_t LINE_BUFFER_SIZE = 128;
constexpr uint8_t PARAMETER_SIZE = 16;

// Keep track when the last time we ran the effect state changes
uint32_t effectPeriodStartMillis = 0;

// WiFI Manager
WiFiManager wm;
#define MQTT_SERVER_LENGTH 40
#define MQTT_PORT_LENGTH 5
#define MQTT_USERNAME_LENGTH 18
#define MQTT_PASSWORD_LENGTH 18
WiFiManagerParameter wm_mqtt_server("server", "mqtt server", "", MQTT_SERVER_LENGTH);
WiFiManagerParameter wm_mqtt_port("port", "mqtt port", "", MQTT_PORT_LENGTH);
WiFiManagerParameter wm_mqtt_user("user", "mqtt username", "", MQTT_USERNAME_LENGTH);

const char _customHtml_hidden[] = "type=\"password\"";
WiFiManagerParameter wm_mqtt_password("input", "mqtt password", "", MQTT_PASSWORD_LENGTH, _customHtml_hidden, WFM_LABEL_AFTER);


// Stores information about the controller on communication level
Properties controllerConfig;
bool controllerConfigModified = false;
// Stores information about the current temperature settings
Properties gaggiaConfig;
bool configModified = false;

// CRC value of last update to MQTT
uint16_t lastMeasurementCRC = 0;
uint32_t shouldRestart = 0;        // Indicate that a service requested an restart. Set to millies() of current time and it will restart 5000ms later

// I/O Sensors and devices
std::unique_ptr<GaggiaClassicController> controller(nullptr);
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

SemaphoreHandle_t xSemaphore = NULL;

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
        configModified = false;
    },
    []() {
        return configModified;
    }
};

OneShot saveHardwareConfigHandler{
    5000,
    []() {
    },
    []() {
        saveConfig(CONTROLLER_CONFIG_FILENAME, controllerConfig);
        controllerConfigModified = false;
    },
    []() {
        return controllerConfigModified;
    }
};

OneShot removeCounterLabel{
    5000,
    []() {
        gaggia_ui_set_visibility(TIMER_BOX, true);
    },
    []() {
        gaggia_ui_set_visibility(TIMER_BOX, false);
    },
    []() {
        bool pump = gaggiaIO.pump();

        if (pump) {
            removeCounterLabel.hold();
        }

        return pump;
    }
};

bool loadConfig(const char* filename, Properties& properties) {
    bool ret = false;

    if (FileSystemFSBegin()) {
        if (FileSystemFS.exists(filename)) {
            //file exists, reading and loading
            File configFile = FileSystemFS.open(filename, "r");

            if (configFile) {
                Serial.print(F("Loading config : "));
                Serial.println(filename);
                deserializeProperties<LINE_BUFFER_SIZE>(configFile, properties);
                serializeProperties<LINE_BUFFER_SIZE>(Serial, properties);
            }

            configFile.close();
        } else {
            Serial.print(F("File not found: "));
            Serial.println(filename);
        }

        // FileSystemFS.end();
    } else {
        Serial.print(F("Failed to begin FileSystemFS"));
    }

    return ret;
}


/**
 * Store custom oarameter configuration in FileSystemFS
 */
bool saveConfig(const char* filename, Properties& properties) {
    bool ret = false;

    if (FileSystemFSBegin()) {
        FileSystemFS.remove(filename);
        File configFile = FileSystemFS.open(filename, "w");

        if (configFile) {
            Serial.print(F("Saving config : "));
            Serial.println(filename);
            serializeProperties<LINE_BUFFER_SIZE>(configFile, properties);
            serializeProperties<LINE_BUFFER_SIZE>(Serial, properties, false);
            ret = true;
        } else {
            Serial.print(F("Failed to write file"));
            Serial.println(filename);
        }

        configFile.close();
    }

    return ret;
}

///////////////////////////////////////////////////////////////////////////
//  Automation
///////////////////////////////////////////////////////////////////////////


void handleScriptContext() {
    int8_t handle = gaggia_scripting_handle();

#if defined (GUI_IO)
    gaggia_ui_set_led(BREW_BUT_STATUS, uiBrewButton);
    gaggia_ui_set_led(STEAM_BUT_STATUS, uiSteamButton);
    gaggia_ui_set_led_bright(HEAT_STATUS_SSR, gaggiaIO.heatElement()->power() * 2);

    if (gaggia_scripting_context() != nullptr) {
        gaggia_ui_set_led(VALVE_STATUS_SSR, gaggia_scripting_context()->m_valve);
        gaggia_ui_set_led(PUMP_STATUS_SSR, gaggia_scripting_context()->m_pump);
    }

#endif

    switch (handle) {
        case 0:
            Serial.println("Script ended");
            gaggiaIO.pump(false);
            gaggiaIO.valve(false);
            controller->brewMode(true);
            controller->setPoint(gaggiaConfig.get("defaultBrewTemp"));
            gaggia_scripting_load("/standby.txt");
            break;

        case 1:
            gaggiaIO.pump(gaggia_scripting_context()->m_pump);
            gaggiaIO.valve(gaggia_scripting_context()->m_valve);
            controller->brewMode(gaggia_scripting_context()->m_brewMode);
            controller->setPoint(gaggia_scripting_context()->m_temperature);
    }
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
    if (controllerConfig.get("statusJson")) {
        static char f[] = "{\"tb\":%.2f,\"ts\":%.2f,\"bo\":%.2f}";
        static char b[sizeof(f) + 3 * 8 + 10]; // 2 bytes per extra item + 10 extra
        format = f;
        buffer = b;
    } else {
        static char f[] = "tb=%.2f ts=%.2f bo=%.2f";
        static char b[sizeof(f) + 3 * 8 + 10]; // 2 bytes per extra item + 10 extra
        format = f;
        buffer = b;
    }


    sprintf(buffer,
            format,
            gaggiaIO.brewTemperature()->get(),
            gaggiaIO.steamTemperature()->get(),
            gaggiaIO.heatElement()->power()
           );


    // Quick hack to only update when data actually changed
    uint16_t thisCrc = crc16((uint8_t*)buffer, std::strlen(buffer));

    if (thisCrc != lastMeasurementCRC) {
        network_publishToMQTT("status", buffer);
        lastMeasurementCRC = thisCrc;
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

    // List all files in base directory
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

///////////////////////////////////////////////////////////////////////////
//  IOHardware
///////////////////////////////////////////////////////////////////////////
void setupIOHardware() {
    controller.reset(new GaggiaClassicController(&gaggiaIO));
    controller->init();
    controller->setPoint(gaggiaConfig.get("defaultBrewTemp"));
    controller->brewMode(true);
}

///////////////////////////////////////////////////////////////////////////
//  Webserver/WIFIManager
///////////////////////////////////////////////////////////////////////////
void saveParamCallback() {
    Serial.println("[CALLBACK] saveParamCallback fired");

    if (std::strlen(wm_mqtt_server.getValue()) > 0) {
        controllerConfig.put("mqttServer", PV(wm_mqtt_server.getValue()));
        controllerConfig.put("mqttPort", PV(std::atoi(wm_mqtt_port.getValue())));
        controllerConfig.put("mqttUsername", PV(wm_mqtt_user.getValue()));
        controllerConfig.put("mqttPassword", PV(wm_mqtt_password.getValue()));
        controllerConfigModified = true;
        // Redirect from MQTT so on the next reconnect we pickup new values
        network_mqtt_disconnect();
        // Send redirect back to param page
        wm.server->sendHeader(F("Location"), F("/param?"), true);
        wm.server->send(302, FPSTR(HTTP_HEAD_CT2), "");   // Empty content inhibits Content-length header so we have to close the socket ourselves.
        wm.server->client().stop();
    }
}

/**
 * Setup the wifimanager and configuration page
 */
void setupWifiManager() {
    char port[6];
    snprintf(port, sizeof(port), "%d", (int16_t)controllerConfig.get("mqttPort"));
    wm_mqtt_port.setValue(port, MQTT_PORT_LENGTH);
    wm_mqtt_password.setValue(controllerConfig.get("mqttPassword"), MQTT_PASSWORD_LENGTH);
    wm_mqtt_user.setValue(controllerConfig.get("mqttUsername"), MQTT_USERNAME_LENGTH);
    wm_mqtt_server.setValue(controllerConfig.get("mqttServer"), MQTT_SERVER_LENGTH);

    wm.addParameter(&wm_mqtt_server);
    wm.addParameter(&wm_mqtt_port);
    wm.addParameter(&wm_mqtt_user);
    wm.addParameter(&wm_mqtt_password);

    /////////////////
    // set country
    wm.setClass("invert");
    wm.setCountry("US"); // setting wifi country seems to improve OSX soft ap connectivity, may help others as well

    // Set configuration portal
    wm.setShowStaticFields(false);
    wm.setConfigPortalBlocking(false); // Must be blocking or else AP stays active
    wm.setDebugOutput(true);
    wm.setSaveParamsCallback(saveParamCallback);
    wm.setHostname(controllerConfig.get("mqttClientID"));
    std::vector<const char*> menu = {"wifi", "wifinoscan", "info", "param", "sep", "erase", "restart"};
    wm.setMenu(menu);

    wm.startWebPortal();
    wm.autoConnect(controllerConfig.get("mqttClientID"));
}

///////////////////////////////////////////////////////////////////////////
//  UI Event handling
///////////////////////////////////////////////////////////////////////////
void setup_ui_events() {
#if defined (GUI_IO)
    gaggia_ui_add_event_cb(STEAM_TEMP_OBJ, [](enum ui_element_types label, enum ui_event event) {
        uiSteamButton = !uiSteamButton;
    });
    gaggia_ui_add_event_cb(BREW_TEMP_OBJ, [](enum ui_element_types label, enum ui_event event) {
        uiBrewButton = !uiBrewButton;
    });
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
        gaggia_scripting_load("/startup.txt");
    });

    gaggia_ui_add_event_cb(BREWTEMP_SPIN, [](enum ui_element_types label, enum ui_event event) {
        gaggiaConfig.put("defaultBrewTemp", PV(gaggia_ui_spin_get_value(BREWTEMP_SPIN) * 1.0f));
        configModified = true;
    });
    gaggia_ui_add_event_cb(STEAMTEMP_SPIN, [](enum ui_element_types label, enum ui_event event) {
        gaggiaConfig.put("defaultSteamTemp", PV(gaggia_ui_spin_get_value(STEAMTEMP_SPIN) * 1.0f));
        configModified = true;
    });
    gaggia_ui_add_event_cb(PROCESS_SELECT_MATRIX, [](enum ui_element_types label, enum ui_event event) {
        char filename[16];
        snprintf(filename, sizeof(filename), "/menu_%d.txt", gaggia_ui_btn_map_active(label));
        gaggia_scripting_load(filename);
        gaggia_ui_change_screen(0);
    });
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
    controllerConfigModified |= controllerConfig.putNotContains("statusJson", PV(true));

    controllerConfig.put("mqttClientID", PV(mqttClientID));
    controllerConfig.put("mqttBaseTopic", PV(mqttBaseTopic));
    controllerConfig.put("mqttLastWillTopic", PV(mqttLastWillTopic));

    // gaggiaConfig
    configModified |= gaggiaConfig.putNotContains("defaultBrewTemp", PV(94.0f));
    configModified |= gaggiaConfig.putNotContains("defaultSteamTemp", PV(145.0f));
    configModified |= gaggiaConfig.putNotContains("powerSaveTemp", PV(50.0f));
}


void displayUpdateTask(void* pvParameters) {
     const TickType_t xDelay = 50 / portTICK_PERIOD_MS;
    while (true) {
        if (xSemaphoreTake(xSemaphore, (TickType_t) 4)) {
            display_loop();
            xSemaphoreGive(xSemaphore);
        }

        vTaskDelay( xDelay );
    }
}

void setup() {
    // Enable serial port
    Serial.begin(115200);
    delay(500);

    pinMode(CONFIG_LV_DISP_SPI_CS, OUTPUT);    // sets the digital pin 13 as output
    digitalWrite(CONFIG_LV_DISP_SPI_CS, LOW);  // sets the digital pin 13 off

    // load configurations
    loadConfig(CONTROLLER_CONFIG_FILENAME, controllerConfig);
    loadConfig(CONFIG_FILENAME, gaggiaConfig);
    setDefaultConfigurations();

    setupIOHardware();
    gaggia_scripting_init(&gaggiaIO);
    gaggia_scripting_load("/startup.txt");

    network_init();
    setupMQTTCallback();
    setupWifiManager();
    display_init();
    setup_ui_events();

    xSemaphore = xSemaphoreCreateMutex();
    xTaskCreatePinnedToCore(
        displayUpdateTask,
        "displayUpdateTask",
        10000,      /* Stack size in words */
        NULL,
        0,
        NULL,
        xPortGetCoreID() ? 0 : 1); /* Pick a core Arduino framework is not using */

    effectPeriodStartMillis = millis();
}

constexpr uint8_t NUMBER_OF_SLOTS = 10;
uint8_t maxSlots = 255;
void loop() {
    const uint32_t currentMillis = millis();

    // Gaggia IO has it's own timer
    gaggiaIO.handle(currentMillis);

    if (currentMillis - effectPeriodStartMillis >= EFFECT_PERIOD_CALLBACK) {

        effectPeriodStartMillis += EFFECT_PERIOD_CALLBACK;
        counter50TimesSec++;

        if (xSemaphoreTake(xSemaphore, (TickType_t) 4)) {
            handleScriptContext();
            xSemaphoreGive(xSemaphore);
        }

        controller -> handle(currentMillis);

        // once a second publish status to mqtt (if there are changes)
        if (counter50TimesSec % 50 == 0) {
            publishStatusToMqtt();
        }

        // Maintenance stuff

#if SHOW_FREE_HEAP
        if (counter50TimesSec % 50 == 0) {
            Serial.println(ESP.getFreeHeap());
        }
#endif

        uint8_t slot50 = 0;

        if (counter50TimesSec % maxSlots == slot50++) {
            network_handle();
        } else if (counter50TimesSec % maxSlots == slot50++) {
            saveHardwareConfigHandler.handle();
        } else if (counter50TimesSec % maxSlots == slot50++) {
            saveGaggiaConfigHandler.handle();
        } else if (counter50TimesSec % maxSlots == slot50++) {
            // Temporary untill we have a better spot

            if (xSemaphoreTake(xSemaphore, (TickType_t) 4)) {
                dtostrf(gaggiaIO.brewTemperature()->get(), 0, 0, gaggia_ui_set_text_buffer(BREW_TEMP_LABEL));
                dtostrf(gaggiaIO.steamTemperature()->get(), 0, 0, gaggia_ui_set_text_buffer(STEAM_TEMP_LABEL));
                gaggia_ui_set_text(BREW_TEMP_LABEL, NULL);
                gaggia_ui_set_text(STEAM_TEMP_LABEL, NULL);
                if (gaggiaIO.pump()) {
                    const float pumpMillis = gaggiaIO.pumpMillis() / 1000.f;
                    if (pumpMillis < 999.0f) {
                        dtostrf(pumpMillis, 1, 1, gaggia_ui_set_text_buffer(TIMER_LABEL));
                        gaggia_ui_set_text(TIMER_LABEL, NULL);
                    }
                }
                xSemaphoreGive(xSemaphore);
            }
        } else if (counter50TimesSec % maxSlots == slot50++) {
            if (xSemaphoreTake(xSemaphore, (TickType_t) 4)) {
                removeCounterLabel.handle();
                xSemaphoreGive(xSemaphore);
            }
        } else if (counter50TimesSec % maxSlots == slot50++) {
            wm.process();
        } else if (counter50TimesSec % maxSlots == slot50++) {
            if (shouldRestart != 0 && (currentMillis - shouldRestart >= 5000)) {
                shouldRestart = 0;
                ESP.restart();
            }
        } else {
            maxSlots = slot50;
        }
    }
}


// //////////////////
// //////////////////
// #include <MAX31855.h>
// /**
//    How Many readings are taken to determine a mean temperature.
//    The more values, the longer a calibration is performed,
//    But the readings will be more accurate.
// */
// /**
//    Delay time between a temperature readings
//    From the temperature sensor (ms).
// */
// #define DELAY_TIME 20
// MAX31855* thermocouple = NULL;

// void setup_() {
//     Serial.begin(115200);
//     delay(1500);
//     thermocouple = new MAX31855(PERI_PIN_SPI_CLK, STEAM_PIN_SPI_CS, PERI_PIN_SPI_MISO);
//     thermocouple->begin();
// }

// void loop_() {
//     thermocouple->read();
//     Serial.print("Temperature:");
//     Serial.print(thermocouple->getInternal());
//     Serial.print(":");
//     Serial.print(thermocouple->getTemperature());
//     Serial.print(":");
//     Serial.print(thermocouple->openCircuit());
//     Serial.print(thermocouple->shortToGND());
//     Serial.print(thermocouple->shortToVCC());
//     Serial.print(thermocouple->genericError());
//     Serial.print(thermocouple->noRead());
//     Serial.println(thermocouple->noCommunication());
//     delay(50);
// }

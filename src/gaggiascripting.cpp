#include <vector>

#include "gaggiascripting.hpp"
#include <scriptrunner.hpp>
#include <gaggiascriptcontext.hpp>
#include <gaggiahwio.hpp>
#include "ui/gaggia_ui.h"
#include "message.hpp"

#include <FS.h>
#include <SPIFFS.h>
#define FileSystemFS SPIFFS
#define FileSystemFSBegin() SPIFFS.begin(true)

/*********************
 *      EXTERNALS
 *********************/
extern Properties gaggiaConfig;
extern QueueHandle_t xUIMessageQueue;

/*********************
 *      Variables
 *********************/

// static GaggiaIO* scripting_gaggiaIO;
static GaggiaScriptContext* scriptContext{nullptr};
static ScriptRunner<GaggiaScriptContext>* scriptRunner = nullptr;
static char scriptContextFileToLoad[32] = ""; // See note for handleScriptContext()
constexpr uint8_t SCRIPT_LABEL_SIZE_MAX = 16;
constexpr uint8_t SCRIPT_LINE_SIZE_MAX = 64;

/*********************
 *      Typedef
 *********************/
typedef UIQueue_message<UIMessage_e, 32> UIMessage_t;
typedef DataPair<ui_element_types, bool> UIVisibility_t;

using namespace rvt::scriptrunner;

bool JumpOrStay(GaggiaScriptContext& context, const char* value, const uint8_t jmpPos) {
    char* jmpLocation = nullptr;
    OptParser::get<SCRIPT_LINE_SIZE_MAX>(value, ',', [&](const OptValue & parsed) {
        if (parsed.pos() == jmpPos) {
            jmpLocation = (char*)parsed;
        }
    });

    if (jmpLocation) {
        context.jump(jmpLocation);
        return true;
    }

    return false;
}

bool getBoolValue(const char* value, uint8_t pos, bool defaultValue = false) {
    bool rValue = defaultValue;
    OptParser::get<SCRIPT_LINE_SIZE_MAX>(value, ',', [&](const OptValue & parsed) {
        if (parsed.pos() == pos) {
            rValue = (bool)parsed;
        }
    });
    return rValue;
}


float getFloatValue(const char* value, uint8_t pos) {
    float rValue = 0.0f;
    OptParser::get<SCRIPT_LINE_SIZE_MAX>(value, ',', [&](const OptValue & parsed) {
        if (parsed.pos() == pos) {
            const char* parsedValue = (char*)parsed;

            if (gaggiaConfig.contains(parsedValue)) {
                rValue = (float)gaggiaConfig.get(parsedValue);
            } else {
                rValue = (float)parsed;
            }
        }
    });
    return rValue;
}

void gaggia_scripting_init(GaggiaIO* gaggiaIO) {
    std::vector<Command<GaggiaScriptContext>*> commands;
    commands.push_back(new Command<GaggiaScriptContext> {"valve", [](const char* value, GaggiaScriptContext & context) {
        context.m_valve = getBoolValue(value, 0);
        return true;
    }
                                                        });

    commands.push_back(new Command<GaggiaScriptContext> {"pump", [](const char* value, GaggiaScriptContext & context) {
        context.m_pump = getBoolValue(value, 0);
        return true;
    }
                                                        });

    commands.push_back(new Command<GaggiaScriptContext> {"brewMode", [](const char* value, GaggiaScriptContext & context) {
        context.m_brewMode = getBoolValue(value, 0);
        return true;
    }
                                                        });

    commands.push_back(new Command<GaggiaScriptContext> {"setTemp", [](const char* value, GaggiaScriptContext & context) {
        context.m_setPoint = getFloatValue(value, 0);
        return true;
    }
                                                        });

    commands.push_back(new Command<GaggiaScriptContext> {"brewTemp", [](const char* value, GaggiaScriptContext & context) {
        context.m_brewMode = true;
        context.m_setPoint = getFloatValue(value, 0);

#if defined (DONT_WAIT_FOR_TEMPS)
        return true;
#else
        return context.m_brewTemperature >= context.m_setPoint;
#endif
    }
                                                        });
    commands.push_back(new Command<GaggiaScriptContext> {"steamTemp", [](const char* value, GaggiaScriptContext & context) {
        context.m_brewMode = false;
        context.m_setPoint = getFloatValue(value, 0);

#if defined (DONT_WAIT_FOR_TEMPS)
        return true;
#else
        return context.m_steamTemperature >= context.m_setPoint;
#endif
    }
                                                        });
    commands.push_back(new Command<GaggiaScriptContext> {"Message", [](const char* value, GaggiaScriptContext & context) {
        Serial.println(value);

        OptParser::get<SCRIPT_LINE_SIZE_MAX>(value, ',', [&](const OptValue & parsed) {
            switch (parsed.pos()) {
                case 0: {

                    UIMessage_t timerMessage1{UIMessage_e::SET_TEXT, PROCESS_MESSAGE_LABEL, (char*)parsed};
                    xQueueSend(xUIMessageQueue, (void*) &timerMessage1, (TickType_t) 1);
                }
                break;

                case 1: {
                    UIMessage_t timerMessage2{UIMessage_e::SET_TEXT, PROCESS_MESSAGE_TITLE, (char*)parsed};
                    xQueueSend(xUIMessageQueue, (void*) &timerMessage2, (TickType_t) 1);
                }
                break;
            }
        });

        UIMessage_t setVisibilityMessage {UIMessage_e::SET_VISIBILITY, PROCESS_MESSAGE_CONTAINER, (bool)true};
        xQueueSend(xUIMessageQueue, (void*) &setVisibilityMessage, (TickType_t) 1);
        return true;
    }
                                                        });

    commands.push_back(new Command<GaggiaScriptContext> {"MessageOff", [](const char* value, GaggiaScriptContext & context) {
        Serial.print("Message Off : ");
        Serial.println(value);

        UIMessage_t setVisibilityMessage {UIMessage_e::SET_VISIBILITY, PROCESS_MESSAGE_CONTAINER, (bool)false};
        xQueueSend(xUIMessageQueue, (void*) &setVisibilityMessage, (TickType_t) 1);
        return true;
    }
                                                        });

    commands.push_back(new Command<GaggiaScriptContext> {"load", [&](const char* value, GaggiaScriptContext & context) {
        Serial.print("Load : ");
        Serial.println(value);
        gaggia_scripting_load(value);
        return false;
    }
                                                        });

    commands.push_back(new Command<GaggiaScriptContext> {"SteamButton", [](const char* value, GaggiaScriptContext & context) {
        if (context.m_steamButton == getBoolValue(value, 0)) {
            return true;
        } else {
            return JumpOrStay(context, value, 1);
        }
    }
                                                        });

    // Wait for Brew Button
    // if not matched optionally jump to location
    commands.push_back(new Command<GaggiaScriptContext> {"BrewButton", [](const char* value, GaggiaScriptContext & context) {
        if (context.m_brewButton == getBoolValue(value, 0)) {
            return true;
        } else {
            return JumpOrStay(context, value, 1);
        }
    }
                                                        });

    // Wait for the brew or steam button is pressed and continue
    // optionally jump to the location of the second parameter of not matched
    commands.push_back(new Command<GaggiaScriptContext> {"BrewOrSteamButton", [](const char* value, GaggiaScriptContext & context) {
        bool buttonValue = getBoolValue(value, 0);

        if (context.m_brewButton == buttonValue) {
            return JumpOrStay(context, value, 1);
        } else if (context.m_steamButton == buttonValue) {
            return JumpOrStay(context, value, 2);
        }

        return getBoolValue(value, 3, false);
    }
                                                        });

    // Wait for both the BrewAndSteamButton are in a specific value
    commands.push_back(new Command<GaggiaScriptContext> {"BrewAndSteamButton", [](const char* value, GaggiaScriptContext & context) {

        bool buttonValue = getBoolValue(value, 0);

        if (context.m_brewButton == buttonValue &&
            context.m_steamButton == buttonValue) {
            return true;
        }

        return false;
    }
                                                        });
    scriptRunner = new ScriptRunner<GaggiaScriptContext> {commands};
}

uint8_t gaggia_load_script() {
    char buffer[1024];

    if (strlen(scriptContextFileToLoad) != 0) {
        if (FileSystemFSBegin()) {
            Serial.print(F("File "));
            Serial.print(scriptContextFileToLoad);

            if (FileSystemFS.exists(scriptContextFileToLoad)) {

                //file exists, reading and loading
                File configFile = FileSystemFS.open(scriptContextFileToLoad, "r");

                if (configFile) {
                    uint16_t pos = 0;

                    while (configFile.available()) {
                        // TODO: Uptimise this with read(uint8_t* buf, size_t size)
                        buffer[pos++] = char(configFile.read());
                    }

                    buffer[pos++] = 0;
                    configFile.close();
                    GaggiaScriptContext* oldContext = scriptContext;

                    if (oldContext != nullptr) {
                        scriptContext = new GaggiaScriptContext{buffer, oldContext};
                    } else {
                        scriptContext = new GaggiaScriptContext{buffer};
                    }

                    delete oldContext;
                    strcpy(scriptContextFileToLoad, "");
                    Serial.println(F(" loaded."));
                    return 2;
                } else {
                    Serial.println(F(" failed."));
                    return 1;
                }
            } else {
                Serial.println(F(" not found."));
                strcpy(scriptContextFileToLoad, "");
                return 1;
            }
        }
    }

    return 0;
}

void gaggia_scripting_load(const char* value) {
    strncpy(scriptContextFileToLoad, value, sizeof(scriptContextFileToLoad));
}

int8_t gaggia_scripting_handle() {
    switch (gaggia_load_script()) {
        // when we just loaded a script, we do need to set the context values first, so we return and let the next run handle that
        case 2:
            return 1;

        case 1:
            // When returned 1 we failed to load or no script was find, so we singled script has ended
            return 0;
    }

    if (scriptContext == nullptr) {
        return -1;
    }

    return scriptRunner->handle(*scriptContext, true);
}

GaggiaScriptContext* gaggia_scripting_context() {
    return scriptContext;
}
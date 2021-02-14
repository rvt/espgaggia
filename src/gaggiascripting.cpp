#include <vector>

#include "gaggiascripting.hpp"
#include <scriptrunner.hpp>
#include <gaggiascriptcontext.hpp>
#include <gaggiahwio.hpp>
#include "ui/gaggia_ui.h"

#include <FS.h>
#include <SPIFFS.h>
#define FileSystemFS SPIFFS
#define FileSystemFSBegin() SPIFFS.begin(true)

/*********************
 *      EXTERNALS
 *********************/
extern Properties gaggiaConfig;
/*********************
 *      Variables
 *********************/

static GaggiaIO* scripting_gaggiaIO;
static GaggiaScriptContext* scriptContext{nullptr};
static CachedScriptRunner<GaggiaScriptContext>* scriptRunner = nullptr;
static char scriptContextFileToLoad[32]; // See note for handleScriptContext()
constexpr uint8_t SCRIPT_LABEL_SIZE_MAX = 16;
constexpr uint8_t SCRIPT_LINE_SIZE_MAX = 64;

using namespace rvt::scriptrunner;

bool OptionalJump(GaggiaScriptContext& context, const char* value, const uint8_t jmpPos) {
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

bool getBoolValue(const char* value, uint8_t pos) {
    bool rValue = false;
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
            rValue = (float)parsed;
        }
    });
    return rValue;
}

void gaggia_scripting_init(GaggiaIO* gaggiaIO) {
    scripting_gaggiaIO = gaggiaIO;
    std::vector<Command<GaggiaScriptContext>*> commands;
    commands.push_back(new Command<GaggiaScriptContext> {"valve", [](const char* value, GaggiaScriptContext & context) {
        context.m_valve = atoi(value);
        return true;
    }
                                                        });

    commands.push_back(new Command<GaggiaScriptContext> {"pump", [](const char* value, GaggiaScriptContext & context) {
        context.m_pump = atoi(value);
        return true;
    }
                                                        });

    commands.push_back(new Command<GaggiaScriptContext> {"brewMode", [](const char* value, GaggiaScriptContext & context) {
        context.m_brewMode = getBoolValue(value, 0);
        return true;
    }
                                                        });

    commands.push_back(new Command<GaggiaScriptContext> {"setTemp", [](const char* value, GaggiaScriptContext & context) {
        float tempValue;

        if (gaggiaConfig.contains(value)) {
            tempValue = (float)gaggiaConfig.get(value);
        } else {
            tempValue = atof(value);
        }

        context.m_temperature = tempValue;
        return true;
    }
                                                        });

    commands.push_back(new Command<GaggiaScriptContext> {"brewTemp", [](const char* value, GaggiaScriptContext & context) {
        context.m_brewMode = true;
        float tempValue;

        if (gaggiaConfig.contains(value)) {
            tempValue = (float)gaggiaConfig.get(value);
        } else {
            tempValue = atof(value);
        }

        context.m_temperature = tempValue;

#if defined (DONT_WAIT_FOR_TEMPS)
        return true;
#else
        return scripting_gaggiaIO->brewTemperature()->get() >= context.m_temperature ;
#endif
    }
                                                        });
    commands.push_back(new Command<GaggiaScriptContext> {"steamTemp", [](const char* value, GaggiaScriptContext & context) {
        context.m_brewMode = true;
        float tempValue;

        if (gaggiaConfig.contains(value)) {
            tempValue = (float)gaggiaConfig.get(value);
        } else {
            tempValue = atof(value);
        }

        context.m_temperature = tempValue;

#if defined (DONT_WAIT_FOR_TEMPS)
        return true;
#else
        return scripting_gaggiaIO->steamTemperature()->get() >= context.m_temperature ;
#endif
    }
                                                        });
    commands.push_back(new Command<GaggiaScriptContext> {"Message", [](const char* value, GaggiaScriptContext & context) {
        Serial.println(value);

        OptParser::get<SCRIPT_LINE_SIZE_MAX>(value, ',', [&](const OptValue & parsed) {
            switch (parsed.pos()) {
                case 0:
                    gaggia_ui_set_text(PROCESS_MESSAGE_LABEL, (char*)parsed);
                    //                    gaggia_ui_set_text(PROCESS_MESSAGE_TITLE, "");
                    break;

                case 1:
                    gaggia_ui_set_text(PROCESS_MESSAGE_TITLE, (char*)parsed);
                    break;
            }
        });

        gaggia_ui_set_visibility(PROCESS_MESSAGE_CONTAINER, true);
        return true;
    }
                                                        });

    commands.push_back(new Command<GaggiaScriptContext> {"MessageOff", [](const char* value, GaggiaScriptContext & context) {
        Serial.println("Message Off");
        gaggia_ui_set_visibility(PROCESS_MESSAGE_CONTAINER, false);
        return true;
    }
                                                        });
    commands.push_back(new Command<GaggiaScriptContext> {"question", [](const char* value, GaggiaScriptContext & context) {

        return true;
    }
                                                        });
    commands.push_back(new Command<GaggiaScriptContext> {"load", [&](const char* value, GaggiaScriptContext & context) {
        strncpy(scriptContextFileToLoad, (char*)value, sizeof(scriptContextFileToLoad));
        return false;
    }
                                                        });

    commands.push_back(new Command<GaggiaScriptContext> {"SteamButton", [](const char* value, GaggiaScriptContext & context) {
        if (scripting_gaggiaIO->steamButton()->current() == getBoolValue(value, 0)) {
            return true;
        } else {
            return OptionalJump(context, value, 1);
        }
    }
                                                        });

    // Wait for Brew Button
    // if not matched optionally jump to location
    commands.push_back(new Command<GaggiaScriptContext> {"BrewButton", [](const char* value, GaggiaScriptContext & context) {
        if (scripting_gaggiaIO->brewButton()->current() == getBoolValue(value, 0)) {
            return true;
        } else {
            return OptionalJump(context, value, 1);
        }
    }
                                                        });

    // Wait for the brew or steam button is pressed and continue
    // optionally jump to the location of the second parameter of not matched
    commands.push_back(new Command<GaggiaScriptContext> {"BrewOrSteamButton", [](const char* value, GaggiaScriptContext & context) {
        bool buttonValue = getBoolValue(value, 0);

        if (scripting_gaggiaIO->brewButton()->current() == buttonValue) {
            return OptionalJump(context, value, 1);
        } else if (scripting_gaggiaIO->steamButton()->current() == buttonValue) {
            return OptionalJump(context, value, 2);
        }

        return OptionalJump(context, value, 3);
    }
                                                        });

    // Wait for both the BrewAndSteamButton are in a specific value
    commands.push_back(new Command<GaggiaScriptContext> {"BrewAndSteamButton", [](const char* value, GaggiaScriptContext & context) {

        bool buttonValue = getBoolValue(value, 0);

        if (scripting_gaggiaIO->steamButton()->current() == buttonValue &&
            scripting_gaggiaIO->brewButton()->current() == buttonValue) {
            return true;
        }

        return false;
    }
                                                        });
    scriptRunner = new CachedScriptRunner<GaggiaScriptContext> {commands};
}

void gaggia_load_script() {
    char buffer[1024];
    uint16_t pos = 0;

    if (FileSystemFS.exists(scriptContextFileToLoad)) {
        //file exists, reading and loading
        File configFile = FileSystemFS.open(scriptContextFileToLoad, "r");

        while (configFile.available()) {
            buffer[pos++] = char(configFile.read());
        }

        buffer[pos++] = 0;
        configFile.close();
        Serial.print(F("Loaded : "));
        Serial.println(scriptContextFileToLoad);
        delete scriptContext;
        scriptContext = new GaggiaScriptContext{scripting_gaggiaIO, buffer};
    } else {
        Serial.print(F("File not found: "));
        Serial.println(scriptContextFileToLoad);
    }

    scriptContextFileToLoad[0] = '\0';
}

void gaggia_scripting_load(const char* value) {
    strncpy(scriptContextFileToLoad, value, sizeof(scriptContextFileToLoad));
}

int8_t gaggia_scripting_handle() {
    if (strlen(scriptContextFileToLoad) != 0) {
        gaggia_load_script();
    } else if (scriptContext == nullptr) {
        return -1;
    }

    return scriptRunner->handle(*scriptContext, true);
}

GaggiaScriptContext* gaggia_scripting_context() {
    return scriptContext;
}
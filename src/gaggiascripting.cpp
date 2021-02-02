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
static ScriptRunner<GaggiaScriptContext>* scriptRunner = nullptr;
static char scriptContextFileToLoad[32]; // See note for handleScriptContext()
constexpr uint8_t SCRIPT_LABEL_SIZE_MAX = 16;
constexpr uint8_t SCRIPT_LINE_SIZE_MAX = 64;

using namespace rvt::scriptrunner;

bool OptionalJump(GaggiaScriptContext& context, const char* value, const uint8_t jmpPos) {
    char* jmpLocation = nullptr;
    OptParser::get<SCRIPT_LINE_SIZE_MAX>(value, ',', [&](const OptValue & value) {
        if (value.pos() == jmpPos) {
            jmpLocation = (char*)value;
        }
    });

    if (jmpLocation) {
        context.jump(jmpLocation);
        return true;
    }

    return false;
}

bool getBoolValue(const OptValue& value, uint8_t pos) {
    bool rValue = false;
    OptParser::get<SCRIPT_LINE_SIZE_MAX>(value, ',', [&](const OptValue & parsed) {
        if (parsed.pos() == pos) {
            rValue = (bool)parsed;
        }
    });
    return rValue;
}

void gaggia_scripting_init(GaggiaIO* gaggiaIO) {
    scripting_gaggiaIO = gaggiaIO;
    std::vector<Command<GaggiaScriptContext>*> commands;
    commands.push_back(new Command<GaggiaScriptContext> {"valve", [](const OptValue & value, GaggiaScriptContext & context) {
        context.m_valve = (bool)value;
        return true;
    }
                                                        });
    commands.push_back(new Command<GaggiaScriptContext> {"pump", [](const OptValue & value, GaggiaScriptContext & context) {
        context.m_pump = (bool)value;
        return true;
    }
                                                        });

    commands.push_back(new Command<GaggiaScriptContext> {"brewTemp", [](const OptValue & value, GaggiaScriptContext & context) {
        context.m_brewMode = true;
        auto tempValue = (float)value;

        if (tempValue < 0) {
            context.m_temperature = (float)gaggiaConfig.get("defaultBrewTemp");
        } else {
            context.m_temperature = tempValue;
        }

#if defined (DONT_WAIT_FOR_TEMPS)
        return true;
#else
        return scripting_gaggiaIO->brewTemperature()->get() >= context.m_temperature ;
#endif
    }
                                                        });
    commands.push_back(new Command<GaggiaScriptContext> {"steamTemp", [](const OptValue & value, GaggiaScriptContext & context) {
        context.m_brewMode = false;
        auto tempValue = (float)value;

        if (tempValue < 0) {
            context.m_temperature = (float)gaggiaConfig.get("defaultSteamTemp");
        } else {
            context.m_temperature = tempValue;
        }

#if defined (DONT_WAIT_FOR_TEMPS)
        return true;
#else
        return scripting_gaggiaIO->steamTemperature()->get() >= context.m_temperature ;
#endif
    }
                                                        });
    commands.push_back(new Command<GaggiaScriptContext> {"Display", [](const OptValue & value, GaggiaScriptContext & context) {
        Serial.println((char*)value);
        gaggia_ui_set_visibility(PROCESS_MESSAGE_CONTAINER, true);
        gaggia_ui_set_text(PROCESS_MESSAGE_LABEL, (char*) value);
        return true;
    }
                                                        });

    commands.push_back(new Command<GaggiaScriptContext> {"PumpMillis", [](const OptValue & value, GaggiaScriptContext & context) {
        gaggia_ui_set_visibility(TIMER_BOX, (bool)value);
        return true;
    }
                                                        });
    commands.push_back(new Command<GaggiaScriptContext> {"Title", [](const OptValue & value, GaggiaScriptContext & context) {
        Serial.println((char*)value);
        gaggia_ui_set_visibility(PROCESS_MESSAGE_CONTAINER, true);
        gaggia_ui_set_text(PROCESS_MESSAGE_TITLE, (char*) value);
        return true;
    }
                                                        });
    commands.push_back(new Command<GaggiaScriptContext> {"TitleOff", [](const OptValue & value, GaggiaScriptContext & context) {
        gaggia_ui_set_text(PROCESS_MESSAGE_TITLE, "");
        return true;
    }
                                                        });

    commands.push_back(new Command<GaggiaScriptContext> {"DisplayOff", [](const OptValue & value, GaggiaScriptContext & context) {
        Serial.println("Display Off");
        gaggia_ui_set_text(PROCESS_MESSAGE_LABEL, "");
        gaggia_ui_set_visibility(PROCESS_MESSAGE_CONTAINER, false);
        return true;
    }
                                                        });
    commands.push_back(new Command<GaggiaScriptContext> {"question", [](const OptValue & value, GaggiaScriptContext & context) {
        if (context.isAdvanced()) {
            // Serial.print(F("Question:"));
            // Serial.println((char*)value);
        }

        return true;
    }
                                                        });
    commands.push_back(new Command<GaggiaScriptContext> {"load", [&](const OptValue & value, GaggiaScriptContext & context) {
        strncpy(scriptContextFileToLoad, (char*)value, sizeof(scriptContextFileToLoad));
        return true;
    }
                                                        });

    commands.push_back(new Command<GaggiaScriptContext> {"SteamButton", [](const OptValue & value, GaggiaScriptContext & context) {
        if (scripting_gaggiaIO->steamButton()->current() == (bool)value) {
            return true;
        } else {
            return OptionalJump(context, value, 1);
        }
    }
                                                        });

    // Wait for Brew Button
    // if not matched optionally jump to location
    commands.push_back(new Command<GaggiaScriptContext> {"BrewButton", [](const OptValue & value, GaggiaScriptContext & context) {
        if (scripting_gaggiaIO->brewButton()->current() == getBoolValue(value, 0)) {
            return true;
        } else {
            return OptionalJump(context, value, 1);
        }
    }
                                                        });

    // Wait for the brew or steam button is pressed and continue
    // optionally jump to the location of the second parameter of not matched
    commands.push_back(new Command<GaggiaScriptContext> {"BrewOrSteamButton", [](const OptValue & value, GaggiaScriptContext & context) {
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
    commands.push_back(new Command<GaggiaScriptContext> {"BrewAndSteamButton", [](const OptValue & value, GaggiaScriptContext & context) {

        bool buttonValue = getBoolValue(value, 0);

        if (scripting_gaggiaIO->steamButton()->current() == buttonValue &&
            scripting_gaggiaIO->brewButton()->current() == buttonValue) {
            return true;
        }

        return false;
    }
                                                        });
    scriptRunner = new ScriptRunner<GaggiaScriptContext> {commands};
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
        // Serial.print(F("File not found: "));
        // Serial.println(scriptContextFileToLoad);
    }

    scriptContextFileToLoad[0] = 0;
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

    if (scriptRunner->handle(*scriptContext)) {
        return 1;
    } else {
        scriptContext = nullptr;
        return 0;
    }
}

GaggiaScriptContext* gaggia_scripting_context() {
    return scriptContext;
}
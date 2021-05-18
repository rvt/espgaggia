#pragma once

#include <scriptrunner.hpp>
#include <button.hpp>
#include <temperaturesensor.hpp>
#include <gaggiaio.hpp>

using namespace rvt::scriptrunner;
typedef PlainTextContext<MAX_SCRIPT_SIZE> PlainTextContextXXX;

class GaggiaScriptContext : public PlainTextContextXXX {
public:
    // Out
    bool m_valve;
    bool m_pump;
    bool m_brewMode;
    float m_setPoint;
    bool m_monitorButtonStop;
    // Read
    float m_brewTemperature;
    float m_steamTemperature;
    bool m_brewButton;
    bool m_steamButton;
    // Internal
    uint16_t m_counter;
    GaggiaScriptContext(
        const char* script
    ) :
        PlainTextContextXXX{script},
        m_valve(false),
        m_pump(false),
        m_brewMode(true),
        m_setPoint(15.0f),
        m_monitorButtonStop(true),
        m_brewTemperature(15.0f),
        m_steamTemperature(15.0f),
        m_brewButton(false),
        m_steamButton(false),
        m_counter(0) {

    }

    GaggiaScriptContext(
        const char* script,
        GaggiaScriptContext* copy
    ) :
        PlainTextContextXXX{script},
        m_valve(copy->m_valve),
        m_pump(copy->m_pump),
        m_brewMode(copy->m_brewMode),
        m_setPoint(copy->m_setPoint),
        m_monitorButtonStop(true),
        m_brewTemperature(copy->m_brewTemperature),
        m_steamTemperature(copy->m_steamTemperature),
        m_brewButton(copy->m_brewButton),
        m_steamButton(copy->m_steamButton),
        m_counter(0) {
    }
};

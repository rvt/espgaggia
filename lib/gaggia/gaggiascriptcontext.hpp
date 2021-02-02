#pragma once

#include <scriptrunner.hpp>
#include <button.hpp>
#include <temperaturesensor.hpp>
#include <gaggiaio.hpp>

using namespace rvt::scriptrunner;
typedef PlainTextContext<512> PlainTextContext512;

class GaggiaScriptContext : public PlainTextContext512 {
public:
    // TODO: Remove m_gaggiaIO so we have a proper place to copy values back and firth from context
    GaggiaIO* m_gaggiaIO;
    bool m_valve;
    bool m_pump;
    float m_temperature;
    bool m_brewMode;

    GaggiaScriptContext(
        GaggiaIO* p_gaggiaIO,
        const char* script) :
        PlainTextContext512{script},
        m_gaggiaIO(p_gaggiaIO),
        m_valve(false),
        m_pump(false),
        m_temperature(20.0f),
        m_brewMode(true) {

    }
};

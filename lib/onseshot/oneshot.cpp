#include "oneshot.hpp"

#ifndef UNIT_TEST
#include <Arduino.h>
#else
extern "C" uint32_t millis();
#endif


OneShot::OneShot(
    const uint32_t p_delayTimeMS,
    CallbackFunction p_startCallback,
    CallbackFunction p_endCallback,
    ModifiedFunction p_modified) :
    m_delayTimeMS{p_delayTimeMS},
    m_startCallback{p_startCallback},
    m_endCallback{p_endCallback},
    m_modified{p_modified},
    m_lastStatus{false},
    m_oneShotStatus{NOP},
    m_startTime{millis()} {
}

void OneShot::handle() {
    const bool status = m_modified();

    if (status && m_lastStatus == false && m_oneShotStatus == NOP) {
        trigger();
    }

    m_lastStatus = status;

    const uint32_t currentMillis = millis();

    if (m_oneShotStatus == END || (m_oneShotStatus == START && (currentMillis - m_startTime >= m_delayTimeMS))) {
        flush();
    }
}

void OneShot::flush() {
    m_endCallback();
    m_oneShotStatus = NOP;
}

bool OneShot::lastStatus() const {
    return m_lastStatus;
}

void OneShot::trigger() {
    m_oneShotStatus = START;
    m_startTime = millis();
    m_startCallback();
}

void OneShot::hold() {
    m_startTime = millis();
}

void OneShot::reset() {
    m_oneShotStatus = NOP;
}

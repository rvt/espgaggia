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
    m_oneShotStatus{WAIT_TRIGGER},
    m_startTime{0},
    m_lastHandleTime{0} {
}

void OneShot::handle(uint32_t currentMillis) {
    const bool status = m_modified();
    m_lastHandleTime = currentMillis;

    if (status && m_oneShotStatus == WAIT_TRIGGER) {
        triggerStart(currentMillis);
    } else if (m_oneShotStatus == STARTED && (currentMillis - m_startTime >= m_delayTimeMS)) {
        triggerEnd();
    }
}

void OneShot::triggerStart(uint32_t currentMillis) {
    m_oneShotStatus = STARTED;
    m_startTime = currentMillis;
    m_startCallback();
}

void OneShot::triggerEnd() {
    m_oneShotStatus = ENDED;
    m_endCallback();
}

void OneShot::reset() {
    if (m_oneShotStatus == ENDED) {
        m_oneShotStatus = STARTED;
        m_startTime = m_lastHandleTime;
    }
}

void OneShot::start() {
    if (m_oneShotStatus == ENDED) {
        m_oneShotStatus = WAIT_TRIGGER;
    }
}

void OneShot::trigger() {
    if (m_oneShotStatus == WAIT_TRIGGER) {
        triggerStart(m_lastHandleTime);
    }
}

void OneShot::stop() {
    m_oneShotStatus = ENDED;
}

void OneShot::hold() {
    if (m_oneShotStatus == STARTED) {
        m_startTime = m_lastHandleTime;
    }
}

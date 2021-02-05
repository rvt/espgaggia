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
    const uint32_t currentMillis = millis(); 
    if (status!=m_lastStatus) {
        m_lastStatus = status; 
        if (status && m_oneShotStatus==NOP) {
            m_oneShotStatus=START;
            m_startTime = currentMillis;
            m_startCallback();
        }
    } 
    if ( m_oneShotStatus == END || (m_oneShotStatus == START && ( currentMillis - m_startTime >= m_delayTimeMS))) {
        m_endCallback();
        m_oneShotStatus = NOP;
    }
}

void OneShot::direct() {
    m_oneShotStatus = END;
}

void OneShot::hold() {
    m_startTime = millis();
}

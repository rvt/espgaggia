#include "gaggiaClassicControllerPID.hpp"

#include <memory>
#include <algorithm>

#ifdef UNIT_TEST
#include <iostream>
#include <iomanip>
#endif
#include <cmath>

// Define the PID tuning Parameters
//constexpr double Kp = 4.5;
//constexpr double Ki = 0.125;
//constexpr double Kd = 0.2;


constexpr double Kp = 3.8;
constexpr double Ki = 0.95;
constexpr double Kd = 0.2;

constexpr uint16_t UPDATES_PER_SECOND = 1;

GaggiaClassicControllerPID::GaggiaClassicControllerPID(GaggiaIO* p_gaggiaIO) :
    m_gaggiaIO(p_gaggiaIO),
    m_periodStartMillis(0),
    m_brewMode(true),
    m_input(20.0),
    m_output(0.0),
    m_setPoint(20.0f) {
    m_pid = new PID{&m_input, &m_output, &m_setPoint, Kp, Ki, Kd, P_ON_M, DIRECT};
    m_pid->SetOutputLimits(0.0, 100.0);
    m_pid->SetSampleTime(1000);
    m_pid->SetMode(AUTOMATIC);
}

GaggiaClassicControllerPID::~GaggiaClassicControllerPID() {
    delete m_pid;
}

void GaggiaClassicControllerPID::setPoint(float setTemp) {
    m_setPoint = setTemp;
}

float GaggiaClassicControllerPID::setPoint() const {
    return m_setPoint;
}

void GaggiaClassicControllerPID::handle(const uint32_t millis) {
    if (millis - m_periodStartMillis < (1000 / UPDATES_PER_SECOND)) {
        return;
    }

    m_periodStartMillis = millis;

    if (m_brewMode) {
        m_input = m_gaggiaIO->brewTemperature()->get();
    } else {
        m_input = m_gaggiaIO->steamTemperature()->get();
    }

    m_pid->Compute();

    if ((m_setPoint - m_input) > 20.0) {
        m_gaggiaIO->boilerSet(100.0);
    } else {
        m_gaggiaIO->boilerSet(m_output);
    }
}

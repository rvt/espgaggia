
#pragma once

#include <memory>
#include <array>

#include "boiler.hpp"
#include "temperaturesensor.hpp"
#include "heatelement.hpp"
#include "gaggiaio.hpp"
#include <PID_v1.h>

#include <cstdlib>
#include <algorithm>

class GaggiaClassicControllerPID : public Boiler {
private:
    GaggiaIO* m_gaggiaIO;
    long m_periodStartMillis;
    bool m_brewMode = true;
    double m_input;
    double m_output;
    double m_setPoint;
    PID* m_pid;
public:
    GaggiaClassicControllerPID(GaggiaIO* p_gaggiaIO);
    virtual ~GaggiaClassicControllerPID();
    /**
     * Very important, call this once in 5 seconds
     */
    virtual void handle(const uint32_t millis);
    virtual void setPoint(float temperature);
    void brewMode(bool brewMode) {
        m_brewMode = brewMode;
    }
    bool brewMode() const {
        return m_brewMode;
    }
    virtual float setPoint() const;

    virtual const char* name() const {
        return "PID";
    }

private:

};

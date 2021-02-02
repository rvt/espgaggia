#pragma once
#include "testtemperature.hpp"


class TestTemperature : public TemperatureSensor {
private:
private:
    float m_temperature;
public:
    TestTemperature() : TemperatureSensor(), m_temperature(80) {

    }
    virtual void set(float t) {
        m_temperature = t;
    };
    virtual float get() const {
        return m_temperature;
    }
    virtual void handle() {

    }

};

#pragma once

#include <utils.h>

class HeatElement {
private:
    float m_power;
    bool m_on;
public:
    HeatElement() :
        m_power(0.0f),
        m_on(true) {
    }

    /**
     * Set normal speed operation
     * p_speed: Dutiy cycle 0..100%
     */
    void power(const float p_power) {
        m_power = between(p_power, 0.0f, 100.0f);
        controlPower(power());
    }

    /**
     * Increase the speed of the fan with a specific value
     */
    void increase(const float p_power) {
        power(m_power + p_power);
    }

    /**
     * Increase the speed of the fan with a specific value
     */
    void increasePercent(const float p_power) {
        power(m_power + m_power / 100 * p_power);
    }

    /**
     * Returns current required speed
     * Value is guaranteed to be 0..100
     */
    float power() const {
        return m_on ? m_power : 0.0f;
    }

    void setOn(bool on) {
        m_on = on;
    }

    bool isOn() const {
        return m_on;
    }

    virtual void handle(const uint32_t millis) = 0;

private:
    virtual void controlPower(const float dutyCucle) = 0;
};

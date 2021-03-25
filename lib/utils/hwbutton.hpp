#pragma once

#include <cstdint>
#include <button.hpp>
#include <memory>
#include <bitset>
#include <Arduino.h>



/**
 * HWButton handles input of a simple button connected to a digital pin.
 * It will beable to detect single click, double click or long press edgeUp and edgeDown situations
 * It is protected against debounce
 *
 * Ensure you call the handle function 50 times/sec to handle the single/double click timing correctly
 *
 * button states are captured and remembered for as long we have bit´s in the train of up to 50 ticks
 * After that all states will be reset to 0.
 */
class HWButton : public Button {
private:
    const uint8_t m_pin;
public:
    /**
     * Build a button with standard coviguration
     * Input it standard inverted for normal pull-up configuration
     * param: Pin number
     */
    HWButton(uint8_t p_pin);
    /**
     * Build a button with specific conviguration
     * param: Pin number
     * param: Invert the pin input, when pull-ip stndard config is inverted
     * param: Alpha wilter value default 150, the hihger the value the more filtering on the digital input
     */
    HWButton(uint8_t p_pin, bool p_invert, int16_t p_alpha);

    virtual bool raw() {
        return digitalRead(m_pin);
    };
};



#include "hwbutton.hpp"
#include <cstdint>
#include <algorithm>

#include <Arduino.h>


HWButton::HWButton(uint8_t p_pin) : HWButton(p_pin, true, 150) {
}

HWButton::HWButton(uint8_t p_pin, bool p_invert, int16_t p_alpha) :
    Button(p_invert, p_alpha),
    m_pin(p_pin) {
    pinMode(m_pin, INPUT_PULLUP);
}


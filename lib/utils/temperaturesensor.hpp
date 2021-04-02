#pragma once
#include <cstdint>

class TemperatureSensor {
public:
    virtual float get() const = 0;
    virtual void handle() = 0;
    virtual uint16_t faultCode() const = 0;
};

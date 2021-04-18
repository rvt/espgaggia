#pragma once

class Boiler {

public:
    virtual void handle(const uint32_t millis) = 0;
    virtual void setPoint(float temperature) = 0;
    virtual float setPoint() const = 0;
    virtual const char* name() const = 0;

    virtual void brewMode(bool brewMode) = 0;
    virtual bool brewMode() const = 0;
};

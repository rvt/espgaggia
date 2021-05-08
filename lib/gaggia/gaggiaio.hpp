#pragma once

#include <cstdint>
#include <hwbutton.hpp>
#include <heatelement.hpp>
#include <onoffheatelement.hpp>
#include <temperaturesensor.hpp>

class GaggiaIO {
public:

    virtual bool pump() const = 0;
    virtual void pump(bool pump) = 0;
    virtual bool valve()const = 0;
    virtual void valve(bool valve)  = 0;
    virtual Button* steamButton() const = 0;
    virtual Button* brewButton() const = 0;
    virtual uint32_t pumpMillis() const = 0;
    virtual TemperatureSensor* brewTemperature() const = 0;
    virtual TemperatureSensor* steamTemperature() const = 0;

    virtual void boilerIncrease(float boiler) = 0;
    virtual void boilerSet(float boiler) = 0;
    virtual HeatElement* heatElement() const = 0;

    virtual void handle(unsigned long millis)  = 0;

    virtual bool stop() const = 0;
    virtual void resetStop();
};

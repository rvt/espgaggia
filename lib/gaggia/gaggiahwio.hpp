#pragma once

#include <Arduino.h>
#include <gaggiaio.hpp>
#include <button.hpp>
#include <heatelement.hpp>
#include <onoffheatelement.hpp>
#include <temperaturesensor.hpp>
#include <max31855sensor.h>
#include <button.hpp>
#include <testknob.hpp>

#if defined (GUI_IO)
extern bool uiBrewButton;
extern bool uiSteamButton;
#endif

class GaggiaHWIO : public GaggiaIO {
    const uint8_t m_pumpPin;
    const uint8_t m_valvePin;

    bool m_pump;
    bool m_valve;
    bool m_stop;
    float m_boilerIncreaseValue;
    float m_boilerSetValue;
    unsigned long m_startMilis;
    uint32_t m_pumpStartTime;
    uint32_t m_pumpStopTime;

    HeatElement* m_heatElement;

    TemperatureSensor* m_brewSensor;
    TemperatureSensor* m_steamSensor;
    Button* m_steamButton;
    Button* m_brewButton;

public:
    GaggiaHWIO(
        uint8_t SPI_CLK_PIN,
        uint8_t SPI_SDO_PIN,
        uint8_t SPI_BREW_CS_PIN,
        uint8_t SPI_STEAM_CS_PIN,
        uint8_t BREW_BUTTON_PIN,
        uint8_t STEAM_BUTTON_PIN,
        uint8_t boiler_pin,
        uint8_t pump_pin,
        uint8_t valve_pin
    ) : GaggiaIO(),
        m_pumpPin(pump_pin),
        m_valvePin(valve_pin),
        m_pump(false),
        m_valve(false),
        m_boilerIncreaseValue(0.0f),
        m_boilerSetValue(-1.0f),
        m_startMilis(0),
        m_pumpStartTime(0),
        m_pumpStopTime(0) {
        pinMode(m_pumpPin, OUTPUT);
        pinMode(m_valvePin, OUTPUT);

        m_heatElement = new OnOffHeatElement{boiler_pin, 2000};

        auto* sensor1 = new MAX31855 {SPI_CLK_PIN, SPI_BREW_CS_PIN, SPI_SDO_PIN};
        sensor1->begin();
        m_brewSensor = new MAX31855sensor{sensor1};

        auto* sensor2 = new MAX31855 {SPI_CLK_PIN, SPI_STEAM_CS_PIN, SPI_SDO_PIN};
        sensor2->begin();
        m_steamSensor = new MAX31855sensor{sensor2};

#if defined (GUI_BUTTONS)
        m_steamButton = new TestKnob { []() {
            return uiSteamButton;
        }
                                     };
        m_brewButton  = new TestKnob { []() {
            return uiBrewButton;
        } };
#else
        m_steamButton = new HWButton {STEAM_BUTTON_PIN, true, 300};
        m_brewButton  = new HWButton{BREW_BUTTON_PIN, true, 300};
#endif

    }

    virtual bool pump() const {
        // We only turn the pum on if also the brew button is on
        // This is a savety feature such that you can only brew coffee
        // when you are at the controls
        return m_pump && m_brewButton->current();
    }

    virtual void pump(bool pump) {
        if (pump == m_pump) {
            return;
        }

        if (pump) {
            m_pumpStartTime = millis();
            m_pumpStopTime = 0;
            m_pump = true;
        } else {
            m_pumpStopTime = millis();
            m_pump = false;
        }
    }

    virtual uint32_t pumpMillis() const {
        if (m_pumpStopTime) {
            return m_pumpStopTime - m_pumpStartTime;
        } else {
            return millis() - m_pumpStartTime;
        }
    }

    virtual bool valve() const {
        return m_valve;
    }

    virtual void valve(bool valve) {
        m_valve = valve;
    }

    virtual void boilerIncrease(float boiler) {
        m_boilerIncreaseValue = boiler;
    }

    virtual void boilerSet(float boiler) {
        m_boilerSetValue = between(boiler, 0.0f, 100.0f);
    }

    virtual Button* steamButton() const {
        return m_steamButton;
    }
    virtual Button* brewButton() const {
        return m_brewButton;
    }

    virtual TemperatureSensor* brewTemperature() const {
        return m_brewSensor;
    }

    virtual TemperatureSensor* steamTemperature() const {
        return m_steamSensor;
    }

    virtual HeatElement* heatElement() const {
        return m_heatElement;
    }

    virtual bool stop() const {
        return m_stop;
    }

    virtual void resetStop() {
        m_stop = false;
    }

    void handle(unsigned long millis) {
        constexpr int period = 1000 / 50;

        if (millis - m_startMilis >= period) {
            m_startMilis += period;

            digitalWrite(m_pumpPin, !pump());
            digitalWrite(m_valvePin, !valve());

            m_heatElement->handle(millis);

            if (m_boilerSetValue >= 0.0f) {
                m_heatElement->power(m_boilerSetValue);
                m_boilerSetValue = -1.0f;
            }

            m_heatElement->increase(m_boilerIncreaseValue);
            m_boilerIncreaseValue = 0.0;

            m_brewSensor->handle();
            m_steamSensor->handle();

            m_steamButton->handle();
            m_brewButton->handle();

            m_stop = m_stop || m_brewButton->isEdgeDown() || m_steamButton->isEdgeDown();
        }


    }
private:

};



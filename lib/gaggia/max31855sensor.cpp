#include "max31855sensor.h"
#define MAX31855_ERR_OC  0x1
#define MAX31855_ERR_GND 0x2
#define MAX31855_ERR_VCC 0x4

MAX31855sensor::MAX31855sensor(MAX31855* p_MAX31855) :
    TemperatureSensor(),
    m_MAX31855(p_MAX31855),
    m_lastTemp(-1.0) {
}

void MAX31855sensor::handle() {
    uint8_t fault = m_MAX31855->read();
    float measured = m_MAX31855->getTemperature();
    //float measured = m_MAX31855->getInternal();


    if (fault != 0) {
        //Serial.print (thermocouple->openCircuit());
        //Serial.print (thermocouple->shortToGND());
        //Serial.print (thermocouple->shortToVCC());
        //Serial.print (thermocouple->genericError());
        //Serial.print (thermocouple->noRead());
        //  Serial.println (thermocouple->noCommunication());
        return;
    } else {
        m_lastTemp = m_lastTemp + (measured - m_lastTemp) * 0.1f;
    }

}

float MAX31855sensor::get() const {
    return m_lastTemp;
}

#include <Arduino.h>
#include "Thermistor.h"

void Thermistor::begin() {
    pinMode(this->analogPin, INPUT);
}

double Thermistor::measureResistance() {
    auto measure = analogRead(this->analogPin);

    if (measure) {
        float voltage = (measure * this->inputVoltage) / 1024.0;
        return this->maxResistance * ((this->inputVoltage / voltage) - 1);
    } else {
        return NULL;
    }
}

double Thermistor::celsius() {
    auto measuredResistance = this->measureResistance();
    if (measuredResistance == NULL) {
        return NULL;
    }

    double logResistances = log(measuredResistance / this->maxResistance);
    auto value = 1.0 / ((1.0 / this->defaultTemp) + ((1.0 / this->betaValue) * logResistances)) - 273.15;
    return value;
}

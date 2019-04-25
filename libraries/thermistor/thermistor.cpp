#include <Arduino.h>
#include "thermistor.h"

void Thermistor::begin() {
    pinMode(this->analogPin, INPUT);
}

double Thermistor::measureResistance() {
    double measure = analogRead(this->analogPin);

    if (measure) {
        double voltage = (1024.0 / measure) - 1.0;
        return this->seriesResistance * voltage;
    } else {
        return NULL;
    }
}

double Thermistor::celsius() {
    auto measuredResistance = this->measureResistance();
    if (measuredResistance == NULL) {
        return NULL;
    }

    float steinhart = measuredResistance / this->nominalResistance;
    steinhart = log(steinhart);
    steinhart /= this->betaValue;
    steinhart += 1.0 / this->defaultTemp;
    steinhart = 1.0 / steinhart;
    steinhart -= 273.15;

    return steinhart;
}

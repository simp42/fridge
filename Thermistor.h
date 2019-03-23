#ifndef AUTOFRIDGE_THERMISTOR_H
#define AUTOFRIDGE_THERMISTOR_H

#include "Arduino.h"

class Thermistor {
public:
    Thermistor(int analogPin, double nominalResistance, double seriesResistance, double betaValue) {
        this->analogPin = analogPin;
        this->nominalResistance = nominalResistance;
        this->seriesResistance = seriesResistance;
        this->betaValue = betaValue;
        // Nenntemperatur in K
        this->defaultTemp = 273.15 + 25;
    }

    void begin();
    double celsius();

private:
    int analogPin;
    double betaValue;
    double nominalResistance;
    double seriesResistance;
    double defaultTemp;

    double measureResistance();
};


#endif //AUTOFRIDGE_THERMISTOR_H

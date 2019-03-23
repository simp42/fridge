#ifndef AUTOFRIDGE_THERMISTOR_H
#define AUTOFRIDGE_THERMISTOR_H

#include "Arduino.h"

class Thermistor {
public:
    Thermistor(int analogPin, int maxResistance, int betaValue) {
        this->analogPin = analogPin;
        this->maxResistance = maxResistance;
        this->betaValue = betaValue;
        this->inputVoltage = 5.0;
        // Nenntemperatur in K
        this->defaultTemp = 273.15 + 25;
    }

    void begin();
    double celsius();

    void setInputVoltage(double voltage) {
        this->inputVoltage = voltage;
    }

private:
    double inputVoltage;
    int analogPin;
    int betaValue;
    int maxResistance;
    double defaultTemp;

    double measureResistance();
};


#endif //AUTOFRIDGE_THERMISTOR_H

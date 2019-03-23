#include <Arduino.h>
#include "User_Setup.h"

#include "Thermistor.cpp"
#include "../../../../Applications/Arduino.app/Contents/Java/hardware/tools/avr/lib/gcc/avr/5.4.0/include/stdint-gcc.h"

const uint8_t OUTPUT_PIN_TRANSISTOR = 9;
const uint8_t INPUT_PIN_THERMISTOR = PIN_A2;
const double DESIRED_TEMP_CELSIUS = 10;
const uint8_t LOOPS_PER_MINUTE = 5;
double currentFanSetting = 127;
uint8_t loopsToReset;

#define USESERIAL true

Thermistor therm(INPUT_PIN_THERMISTOR, 5000, 3950);

void setFan(int value) {
    if (value >= 255) {
        analogWrite(OUTPUT_PIN_TRANSISTOR, 255);
    }
    else if (value <= 0) {
        analogWrite(OUTPUT_PIN_TRANSISTOR, 0);
    } else {
        analogWrite(OUTPUT_PIN_TRANSISTOR, value);
    }
}

void setup() {
    if (USESERIAL) {
        Serial.begin(9600);
    }

    //therm.setInputVoltage(4.5);
    therm.begin();

    pinMode(LED_BUILTIN, OUTPUT);

    setFan(currentFanSetting);
    // Reset alle 10 Minuten
    loopsToReset = LOOPS_PER_MINUTE * 10;
}

void loop() {
    if (loopsToReset == 0) {
        currentFanSetting = 127;
        loopsToReset = LOOPS_PER_MINUTE * 10;
    } else {
        loopsToReset--;

        if (USESERIAL) {
            Serial.println("------------------------");
            Serial.print("Reset in:");
            Serial.println(loopsToReset);
            Serial.print("Bisherige FAN-Einstellung:");
            Serial.println(currentFanSetting);
        }

        auto temp = therm.celsius();

        if (USESERIAL) {
            Serial.print("Temperatur: ");
            Serial.println(temp);
        }

        // TODO: Ordenltichen Algorithmus für Erreichen/Halten der Zieltemperatur
        // Ableitung der bisherigen Verläufe regeln?

        auto deltaTemp = temp - DESIRED_TEMP_CELSIUS;

        if (USESERIAL) {
            Serial.print("Temperaturdifferenz:");
            Serial.println(deltaTemp);
        }

        auto blinks = 3;
        if (deltaTemp < 0) {
            blinks = 2;
        } else {
            blinks = 5;
        }

        for (auto i = 0; i < blinks; i++) {
            digitalWrite(LED_BUILTIN, HIGH);
            delay(100);
            digitalWrite(LED_BUILTIN, LOW);
            delay(100);
        }

        // Positiv wenn über Zieltemperatur, sonst negativ
        // d.h. wir stellen vorerst einfach die Fan-EInstellung um das Delta um, soweit es geht
        currentFanSetting += deltaTemp;

        if (currentFanSetting < 0) {
            currentFanSetting = 0;
        }
        if (currentFanSetting > 255) {
            currentFanSetting = 255;
        }

        if (deltaTemp > 5) {
            currentFanSetting = 255;
        }
        if (deltaTemp < 5) {
            currentFanSetting = 0;
        }

        if (USESERIAL) {
            Serial.print("Stelle FAN ein:");
            Serial.println(currentFanSetting);
        }
    }

    setFan(currentFanSetting);

    // Für 20 Sekunden schlafen legen
    delay(20000);

}

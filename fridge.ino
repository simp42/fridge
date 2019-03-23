#include <Arduino.h>
#include "User_Setup.h"

#include "Thermistor.cpp"

#define USESERIAL true

const uint8_t OUTPUT_PIN_TRANSISTOR = 9;
const uint8_t INPUT_PIN_THERMISTOR = PIN_A2;
const double DESIRED_TEMP_CELSIUS = 10;
const int LOOPS_PER_MINUTE = 5;
int currentFanSetting = 127;

const int BLINK_PATTERN_OVER_5 = 5;
const int BLINK_PATTERN_UNDER_NEG_5 = 6;
const int BLINK_PATTERN_OVER = 2;
const int BLINK_PATTERN_UNDER = 3;

Thermistor therm(INPUT_PIN_THERMISTOR, 5000, 3950);

/**
 * Set the Cooling FAN speed by adjusting the PWM width
 * @param value
 */
void setFan(int value) {
    if (value >= 255) {
        analogWrite(OUTPUT_PIN_TRANSISTOR, 255);
    } else if (value <= 0) {
        analogWrite(OUTPUT_PIN_TRANSISTOR, 0);
    } else {
        analogWrite(OUTPUT_PIN_TRANSISTOR, value);
    }
}

void setup() {
    if (USESERIAL) {
        Serial.begin(9600);
    }

    therm.setInputVoltage(4.5);
    therm.begin();

    pinMode(LED_BUILTIN, OUTPUT);

    // Set PWM frequency to the slowest value possible to reduce noise
    setPwmFrequency(OUTPUT_PIN_TRANSISTOR, 1024);
    setFan(currentFanSetting);
}

void loop() {
    if (USESERIAL) {
        Serial.println("------------------------");
        Serial.print("Bisherige FAN-Einstellung:");
        Serial.println(currentFanSetting);
    }

    auto temp = therm.celsius();

    if (USESERIAL) {
        Serial.print("Temperatur: ");
        Serial.println(temp);
    }

    // Compute delta between actual temperature and desired state
    auto deltaTemp = temp - DESIRED_TEMP_CELSIUS;

    if (USESERIAL) {
        Serial.print("Temperaturdifferenz:");
        Serial.println(deltaTemp);
    }

    auto timeSkipped = 0;

    if (deltaTemp > 5) {
        // Severely over the desired temperature, go 100% cooling
        currentFanSetting = 255;
        timeSkipped += blinkInternalLed(BLINK_PATTERN_OVER_5);
    }
    else if (deltaTemp < -5) {
        // Severly under the desired temperature
        currentFanSetting = 0;
        timeSkipped += blinkInternalLed(BLINK_PATTERN_UNDER_NEG_5);
    } else if (deltaTemp > 0) {
        currentFanSetting += 10;
        if (currentFanSetting > 255) {
            currentFanSetting = 255;
        }
        timeSkipped += blinkInternalLed(BLINK_PATTERN_OVER);
    } else {
        currentFanSetting -= 10;
        if (currentFanSetting < 0) {
            currentFanSetting = 0;
        }
        timeSkipped += blinkInternalLed(BLINK_PATTERN_UNDER);
    }

    if (USESERIAL) {
        Serial.print("Stelle FAN ein:");
        Serial.println(currentFanSetting);
    }

    setFan(currentFanSetting);

    delay(static_cast<unsigned long>(1000 * (60 / LOOPS_PER_MINUTE) - timeSkipped));
}

#include <Arduino.h>
#include "User_Setup.h"

#include "thermistor.h"

#define USESERIAL false

const uint8_t OUTPUT_PIN_TRANSISTOR = 9;
const uint8_t INPUT_PIN_THERMISTOR = PIN_A2;
const double DESIRED_TEMP_CELSIUS = 10;
const int LOOPS_PER_MINUTE = 3;

const int BLINK_PATTERN_OVER_3 = 5;
const int BLINK_PATTERN_UNDER_NEG_3 = 6;
const int BLINK_PATTERN_OVER = 2;
const int BLINK_PATTERN_UNDER = 3;

int currentFanSetting = 127;
double previousDelta;

Thermistor therm(INPUT_PIN_THERMISTOR, 5000, 7500, 3950);

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

    auto timeSkipped = 0;

    double sumTemps = 0;
    const int TEMP_MEASUREMENTS = 3;
    for(int i = 0; i < TEMP_MEASUREMENTS; i++) {
        double measurement = therm.celsius();
        sumTemps += measurement;
        delay(200);
        timeSkipped += 200;
        if (USESERIAL) {
            Serial.print("... Messung: ");
            Serial.println(measurement);
        }

    }
    auto temp = sumTemps / TEMP_MEASUREMENTS;

    // Debug - blink current measured temperature
    timeSkipped += blinkInternalLed(temp);
    delay(1000);
    timeSkipped += 1000;

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

    if (deltaTemp > 3) {
        // Severely over the desired temperature, go 100% cooling
        currentFanSetting = 255;
        timeSkipped += blinkInternalLed(BLINK_PATTERN_OVER_3);
    }
    else if (deltaTemp < -3) {
        // Severly under the desired temperature
        currentFanSetting = 0;
        timeSkipped += blinkInternalLed(BLINK_PATTERN_UNDER_NEG_3);
    } else if (deltaTemp > 0) {
        if (deltaTemp >= previousDelta) {
            currentFanSetting += 10;
            if (currentFanSetting > 255) {
                currentFanSetting = 255;
            }
        } else if (deltaTemp < 1) {
            currentFanSetting -= 10;
            if (currentFanSetting < 0) {
                currentFanSetting = 0;
            }
        }
        timeSkipped += blinkInternalLed(BLINK_PATTERN_OVER);
    } else {
        if (deltaTemp <= previousDelta) {
            currentFanSetting -= 10;
            if (currentFanSetting < 0) {
                currentFanSetting = 0;
            }
        } else if (deltaTemp > 1) {
            currentFanSetting += 10;
            if (currentFanSetting > 255) {
                currentFanSetting = 255;
            }
        }
        timeSkipped += blinkInternalLed(BLINK_PATTERN_UNDER);
    }

    previousDelta = deltaTemp;

    if (USESERIAL) {
        Serial.print("Stelle FAN ein:");
        Serial.println(currentFanSetting);
    }

    setFan(currentFanSetting);

    delay(static_cast<unsigned long>(1000 * (60 / LOOPS_PER_MINUTE) - timeSkipped));
}

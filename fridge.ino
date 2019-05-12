#include <Arduino.h>
#include <LiquidCrystal.h>
#include <PushButton.h>

#include "User_Setup.h"
#include "thermistor.h"
#include "PushButton.h"

#define USESERIAL false

const uint8_t OUTPUT_PIN_TRANSISTOR = 9;
const uint8_t INPUT_PIN_THERMISTOR = PIN_A2;

const uint8_t INPUT_PIN_BUTTON_PLUS = PIN_A5;
const uint8_t INPUT_PIN_BUTTON_MINUS = PIN_A4;

const int LOOPS_PER_MINUTE = 2;

int currentFanSetting = 127;
double currentTemperatureC = -9999;
double desiredTemperatureC = 4;
double previousDelta;

unsigned long millisForNextCheck = 0;

Thermistor therm(INPUT_PIN_THERMISTOR, 5000, 5000, 3950);

// initialize the LCD library by associating any needed LCD interface pin
// with the arduino pin number it is connected to
LiquidCrystal lcd(12, 11, 5, 4, 3, 2);

auto plusButton = new PushButton{
        INPUT_PIN_BUTTON_PLUS,
        [](PushButton* button) {
            if (button->isButtonPushed()) {
                if (desiredTemperatureC < 50) {
                    desiredTemperatureC += 1.0;
                }

                if (USESERIAL) {
                    Serial.print("New temperature: ");
                    Serial.println(desiredTemperatureC);
                }

                updateLcd();

                // Reset debounce state to allow for keeping the button pushed
                button->resetDebounceDelay(500);
            }
        }
};
auto minusButton = new PushButton{
        INPUT_PIN_BUTTON_MINUS,
        [](PushButton* button) {
            if (button->isButtonPushed()) {
                if (desiredTemperatureC > 0) {
                    desiredTemperatureC -= 1.0;
                }

                if (USESERIAL) {
                    Serial.print("New temperature: ");
                    Serial.println(desiredTemperatureC);
                }

                updateLcd();

                // Reset debounce state to allow for keeping the button pushed
                button->resetDebounceDelay(500);
            }
        }
};

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

/**
 * Print information to the LCD
 */
void updateLcd() {
    if (currentTemperatureC <= -9998) {
        return;
    }

    lcd.clear();

    lcd.setCursor(1, 0);
    lcd.print(currentTemperatureC);
    lcd.write(byte(0)); // custom degree char

    lcd.setCursor(0, 1);
    lcd.print((int)round(desiredTemperatureC));
    lcd.write(byte(0)); // custom degree char

    lcd.write(" ");
    auto pwmPercent = (int)round((currentFanSetting / 255.0) * 100);
    lcd.print(pwmPercent);
    lcd.print("%");
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

    // set up the LCD's number of columns and rows:
    lcd.begin(8, 2);
    lcd.print("starting");
    lcd.setCursor(0, 1);
    lcd.print("........");

    lcd.createChar(0, CUSTOM_CHAR_DEGREE);

    millisForNextCheck = millis() + 1000;
}

void loop() {
    // Check the push button states with every iteration
    plusButton->checkButton();
    minusButton->checkButton();

    if (millis() >= millisForNextCheck) {
        lcd.noDisplay();

        if (USESERIAL) {
            Serial.println("------------------------");
            Serial.print("Desired Temperature:");
            Serial.println(desiredTemperatureC);
            Serial.print("Current fan setting:");
            Serial.println(currentFanSetting);
        }

        auto timeSkipped = blinkInternalLed(2);

        double sumTemps = 0;
        const int TEMP_MEASUREMENTS = 3;
        for (int i = 0; i < TEMP_MEASUREMENTS; i++) {
            double measurement = therm.celsius();
            sumTemps += measurement;
            delay(1000);
            timeSkipped += 1000;
            if (USESERIAL) {
                Serial.print("... Measurement: ");
                Serial.println(measurement);
            }

        }
        currentTemperatureC = sumTemps / TEMP_MEASUREMENTS;

        if (USESERIAL) {
            Serial.print("Temperature: ");
            Serial.println(currentTemperatureC);
        }

        // Compute delta between actual temperature and desired state
        auto deltaTemp = currentTemperatureC - desiredTemperatureC;

        if (USESERIAL) {
            Serial.print("Temperature delta:");
            Serial.println(deltaTemp);
        }

        if (deltaTemp > 3) {
            // Severely over the desired temperature, go 100% cooling
            currentFanSetting = 255;
        } else if (deltaTemp < -3) {
            // Severly under the desired temperature
            currentFanSetting = 0;
        } else if (deltaTemp > 0) {
            if (deltaTemp >= previousDelta) {
                currentFanSetting += 5;
                if (currentFanSetting > 255) {
                    currentFanSetting = 255;
                }
            } else if (deltaTemp < 1) {
                currentFanSetting -= 5;
                if (currentFanSetting < 0) {
                    currentFanSetting = 0;
                }
            }
        } else {
            if (deltaTemp <= previousDelta) {
                currentFanSetting -= 5;
                if (currentFanSetting < 0) {
                    currentFanSetting = 0;
                }
            } else if (deltaTemp > 1) {
                currentFanSetting += 5;
                if (currentFanSetting > 255) {
                    currentFanSetting = 255;
                }
            }
        }

        previousDelta = deltaTemp;

        if (USESERIAL) {
            Serial.print("New fan setting:");
            Serial.println(currentFanSetting);
        }

        setFan(currentFanSetting);

        updateLcd();
        lcd.display();

        millisForNextCheck = millis() + 1000 * (60 / LOOPS_PER_MINUTE) - timeSkipped;
    }

    delay(10);
}

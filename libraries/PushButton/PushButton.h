#ifndef FRIDGE_PUSHBUTTON_H
#define FRIDGE_PUSHBUTTON_H

class PushButton {
public:
    PushButton(uint8_t pin, int pushedButtonState) {
        this->pin = pin;
        this->pushedButtonState = pushedButtonState;
        this->debounceMillisDelay = 100;
        this->previousButtonState = -1;
        this->buttonPushed = false;
        this->handler = nullptr;

        pinMode(pin, INPUT);

        resetDebounceDelay();
    }

    PushButton(uint8_t pin) : PushButton(pin, HIGH) {}

    PushButton(uint8_t pin, void (*pushHandler)(PushButton *button)) : PushButton(pin, HIGH) {
        this->RegisterHandler(pushHandler);
    }

    const int getGebounceMilis() {
        return this->debounceMillisDelay;
    }

    void setDebounceMillis(const int millis) {
        this->debounceMillisDelay = millis;
    }

    void checkButton();

    const bool isButtonPushed() { return this->buttonPushed; }

    void RegisterHandler(void (*pushHandler)(PushButton *button)) {
        this->handler = pushHandler;
    }

    void resetDebounceDelay() {
        resetDebounceDelay(0);
    }

    void resetDebounceDelay(int additionalDelay);

private:
    uint8_t pin;
    int pushedButtonState;
    int debounceMillisDelay;
    unsigned long debouncedAfter;
    int previousButtonState;
    bool buttonPushed;

    void (*handler)(PushButton *button);
};


#endif //FRIDGE_PUSHBUTTON_H

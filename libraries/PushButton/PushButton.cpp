#include <USBAPI.h>
#include "PushButton.h"

void PushButton::checkButton() {
    auto currentButtonState = digitalRead(this->pin);

    if (previousButtonState == -1 ||
        currentButtonState != this->previousButtonState) {
        // Either we don't have a previous button state or the state changed
        // So we reset the time in millis to wait for a state change again

        // Set the time after which we assume the button debounced
        this->resetDebounceDelay();
    }

    if (millis() > this->debouncedAfter) {
        // State was constant for the debounce delay, accept this as current state
        this->buttonPushed = (currentButtonState == this->pushedButtonState);
        // Set this to max value for unsigned long, so only a state change will lead to a new debounced state
        this->debouncedAfter = 4294967295;

        // Call push event handler, if one is registered
        // In this case we also throw away the current button state to make this function one-shot
        if (this->handler != nullptr) {
            this->handler(this);
        }
    }

    this->previousButtonState = currentButtonState;
}

void PushButton::resetDebounceDelay(int additionalDelay) {
    this->debouncedAfter = millis() + debounceMillisDelay + additionalDelay;
}

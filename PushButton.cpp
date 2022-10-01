#include "PushButton.h"

PushButton::PushButton(int Num) {
    Pin = Num;
    pinMode(Pin, INPUT_PULLUP);
}

bool PushButton::isPressed(void) {
    bool state;
    int val;
    val = digitalRead(Pin);
    if(val == LOW) state = true;
    else state = false;
    return state;
}

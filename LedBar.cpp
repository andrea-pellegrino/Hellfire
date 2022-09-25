#include "LedBar.h"

LedBar::LedBar(byte* pinArray, int Num) {
        for (int i=0; i<Num; i++) pinMode(*(pinArray+i), OUTPUT);
        StartTime = millis();
        Array = pinArray;
        Size = Num;
        State = 0;
        isUp = true;
}

bool LedBar::Inc(void) {
    bool err;
    if ((State >= 0) && (State < Size)) {
        digitalWrite(*(Array+State), HIGH);
        State++;
        err = false;
    }
    else err = true;
    return err;
}

bool LedBar::Dec(void) {
    bool err;
    if ((State > 0) && (State <= Size)) {
        State--;
        digitalWrite(*(Array+State), LOW);
        err = false;
    }
    else err = true;
    return err;
}

void LedBar::Blink(int Period_ms) {
    CurrTime = millis();
    if ((CurrTime - StartTime) >= Period_ms) {
        if (isUp) {
            if (Inc()) isUp = false;
        }
        else {
            if (Dec()) isUp = true;
        }
        StartTime = CurrTime;
    }
}

void LedBar::Reset(void) {
    int i;
    for (i=0; i<Size; i++) digitalWrite(*(Array+i), LOW);
    State = 0;
}

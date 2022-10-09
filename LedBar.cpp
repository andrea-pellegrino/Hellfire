#include "LedBar.h"

LedBar::LedBar(byte* pinArray, int Num) {
        for (int i=0; i<Num; i++) pinMode(*(pinArray+i), OUTPUT);
        StartTime = millis();
        Array = pinArray;
        Size = Num;
        State = 0;
        isUp = true;
}

bool LedBar::inc(void) {
    bool err;
    if ((State >= 0) && (State < Size)) {
        digitalWrite(*(Array+State), HIGH);
        State++;
        err = false;
    }
    else err = true;
    return err;
}

bool LedBar::dec(void) {
    bool err;
    if ((State > 0) && (State <= Size)) {
        State--;
        digitalWrite(*(Array+State), LOW);
        err = false;
    }
    else err = true;
    return err;
}

void LedBar::blink(int Period_ms) {
    CurrTime = millis();
    if ((CurrTime - StartTime) >= Period_ms) {
        if (isUp) {
            if (inc()) isUp = false;
        }
        else {
            if (dec()) isUp = true;
        }
        StartTime = CurrTime;
    }
}

void LedBar::reset(void) {
    int i;
    for (i=0; i<Size; i++) digitalWrite(*(Array+i), LOW);
    State = 0;
}

void LedBar::set(int Val) {
    int i;
    int limit;
    if (Val >= Size) limit = Size;
    else limit = Val;
    for (i=0; i<limit; i++) digitalWrite(*(Array+i), HIGH);
    State = Val;
}

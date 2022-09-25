#include "TimeCount.h"

TimeCount::TimeCount(void) {

}

void TimeCount::SetTime(unsigned int _Time_s) {
    Time_s = _Time_s;
    Time_ms = millis();
}

unsigned int TimeCount::GetTime(void) {
    return Time_s;
}

void TimeCount::Inc(void) {}
void TimeCount::Dec(void) {}

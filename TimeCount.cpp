#include "TimeCount.h"

TimeCount::TimeCount(void) {

}

void TimeCount::setTime(unsigned int _Time_s) {
    Time_s = _Time_s;
    Time_ms = millis();
    Time_last_ms = millis();
}

bool TimeCount::count(void) {
    bool clock;
    Time_ms = millis();
    if ((Time_ms - Time_last_ms) >= 1000) {
        Time_last_ms = Time_ms;
        clock = true;
    }
    else clock = false;
    return clock;
}

unsigned int TimeCount::inc(void) {
    Time_s++;
    return Time_s;
}
unsigned int TimeCount::dec(void) {
    if (Time_s > 0) Time_s--;
    return Time_s;
}

char* TimeCount::str(void) {
    unsigned int sec, min, hour;
    hour = Time_s/3600;
    min = (Time_s%3600)/60;
    sec = Time_s%60;
    snprintf(Timestring, 9, "%02u:%02u:%02u", hour, min, sec);
    return Timestring;
}

#include "TimeCount.h"

TimeCount::TimeCount(void) {

}

void TimeCount::SetTime(unsigned int _Time_s) {
    Time_s = _Time_s;
    Time_ms = millis();
}

unsigned int TimeCount::Inc(void) {
    Time_ms = millis();
    if ((Time_ms - Time_last_ms) >= 1000) {
        Time_s++;
        Time_last_ms = Time_ms;
    }
    return Time_s;
}
unsigned int TimeCount::Dec(void) {
    Time_ms = millis();
    if (((Time_ms - Time_last_ms) >= 1000) && (Time_s > 0)) {
        Time_s--;
        Time_last_ms = Time_ms;
    }
    return Time_s;
}

char* TimeCount::Str(void) {
    unsigned int sec, min, hour;
    hour = Time_s/3600;
    min = (Time_s%3600)/60;
    sec = Time_s%60;
    snprintf(Timestring, 9, "%02u:%02u:%02u", hour, min, sec);
    return Timestring;
}

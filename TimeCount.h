#ifndef TIMECOUNT_H
#define TIMECOUNT_H

#include "Arduino.h"

class TimeCount {
    public:
        TimeCount(void);
        bool count(void);
        void setTime(unsigned int);
        unsigned int inc(void);
        unsigned int dec(void);
        char* str(void);
    private:
        unsigned int Time_s;
        unsigned long Time_ms;
        unsigned long Time_last_ms;
        char Timestring[9];
};

#endif

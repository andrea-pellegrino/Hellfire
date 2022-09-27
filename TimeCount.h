#ifndef TIMECOUNT_H
#define TIMECOUNT_H

#include "Arduino.h"

class TimeCount {
    public:
        TimeCount(void);
        void SetTime(unsigned int);
        unsigned int Inc(void);
        unsigned int Dec(void);
        char* Str(void);
    private:
        unsigned int Time_s;
        unsigned long Time_ms;
        unsigned long Time_last_ms;
        char Timestring[9];
};

#endif

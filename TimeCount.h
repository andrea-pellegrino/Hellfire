#ifndef TIMECOUNT_H
#define TIMECOUNT_H

#include "Arduino.h"

class TimeCount {
    public:
        TimeCount(void);
        void SetTime(unsigned int);
        unsigned int GetTime(void);
        void Inc(void);
        void Dec(void);
    private:
        unsigned int Time_s;
        unsigned long Time_ms;
};

#endif

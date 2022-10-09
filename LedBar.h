#ifndef LEDBAR_H
#define LEDBAR_H

#include "Arduino.h"

class LedBar {
    public:
        LedBar(byte*, int);
        bool inc(void);
        bool dec(void);
        void blink(int);
        void reset(void);
        void set(int);
    private:
        unsigned long StartTime;
        unsigned long CurrTime;
        byte* Array;
        int Size;
        int State;
        bool isUp;
};

#endif

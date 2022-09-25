#ifndef LEDBAR_H
#define LEDBAR_H

#include "Arduino.h"

class LedBar {
    public:
        LedBar(byte*, int);
        bool Inc(void);
        bool Dec(void);
        void Blink(int);
        void Reset(void);
    private:
        unsigned long StartTime;
        unsigned long CurrTime;
        byte* Array;
        int Size;
        int State;
        bool isUp;
};

#endif

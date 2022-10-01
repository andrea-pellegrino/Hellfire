#ifndef PUSHBUTTON_H
#define PUSHBUTTON_H

#include "Arduino.h"

class PushButton {
    public:
        PushButton(int);
        bool isPressed(void);
    private:
        int Pin;
};

#endif

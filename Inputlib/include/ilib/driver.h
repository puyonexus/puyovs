#pragma once

#include "inputlib.h"
#include "inputevent.h"

namespace ilib {

class Driver;
Driver *getDriver();

class Driver : NonCopyable
{
public:
    struct Gamepad
    {
        virtual ~Gamepad() { }

        virtual int numButtons() = 0;
        virtual int numAxis() = 0;
        virtual int numHats() = 0;

        virtual bool button(int num) = 0;
        virtual float axis(int num) = 0;
        virtual HatPosition hat(int num) = 0;
    };

    virtual ~Driver() {};

    virtual void process() = 0;
    virtual bool error() = 0;

    virtual void enableEvents() = 0;
    virtual void disableEvents() = 0;
    virtual bool getEvent(InputEvent *e) = 0;
};

}

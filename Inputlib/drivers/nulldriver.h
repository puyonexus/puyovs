#pragma once

#include "inputlib.h"
#include "driver.h"

namespace ilib {

class NullDriver : public Driver
{
public:
    NullDriver();
    ~NullDriver();

    bool error();
    void process();

    void enableEvents();
    void disableEvents();
    bool getEvent(InputEvent *e);
};

}

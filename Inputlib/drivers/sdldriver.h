#pragma once

#include "inputlib.h"
#include "driver.h"

namespace ilib {

class SDLDriver : public Driver
{
    ILIB_DECLARE_PRIV;

public:
    SDLDriver();
    ~SDLDriver();

    bool error();
    void process();

    void enableEvents();
    void disableEvents();
    bool getEvent(InputEvent *e);


};

}

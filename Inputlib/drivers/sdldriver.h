#pragma once

#include "inputlib.h"
#include "driver.h"

namespace ilib {

class SDLDriver : public Driver
{
    ILIB_DECLARE_PRIV;

public:
    SDLDriver();
    ~SDLDriver() override;

    bool error() const override;
    void process() override;

    void enableEvents() override;
    void disableEvents() override;
    bool getEvent(InputEvent *e) override;


};

}

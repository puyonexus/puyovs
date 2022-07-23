#pragma once

#include "inputlib.h"
#include "driver.h"

namespace ilib {

class NullDriver : public Driver
{
public:
    NullDriver();
    ~NullDriver() override;

    bool error() const override;
    void process() override;

    void enableEvents() override;
    void disableEvents() override;
    bool getEvent(InputEvent *e) override;
};

}

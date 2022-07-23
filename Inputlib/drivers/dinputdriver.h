#pragma once

#include "inputlib.h"
#include "driver.h"

namespace ilib {

class DInputDriver : public Driver
{
    ILIB_DECLARE_PRIV;

public:
    DInputDriver();
    ~DInputDriver() override;

    bool error() const override;
    void process() override;

    void enableEvents() override;
    void disableEvents() override;
    bool getEvent(InputEvent *e) override;


};

}

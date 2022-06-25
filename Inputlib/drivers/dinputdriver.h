#ifndef DINPUTDRIVER_H
#define DINPUTDRIVER_H

#include "inputlib.h"
#include "driver.h"

namespace ilib {

class DInputDriver : public Driver
{
    ILIB_DECLARE_PRIV;

public:
    DInputDriver();
    ~DInputDriver();

    bool error();
    void process();

    void enableEvents();
    void disableEvents();
    bool getEvent(InputEvent *e);


};

}

#endif // DINPUTDRIVER_H

#ifndef NULLDRIVER_H
#define NULLDRIVER_H

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

#endif // NULLDRIVER_H

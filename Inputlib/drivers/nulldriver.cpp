#include "nulldriver.h"

namespace ilib {

NullDriver::NullDriver() = default;
NullDriver::~NullDriver() = default;

bool NullDriver::error() const
{
    // NullDriver: 100% bug and failure free!
    return false;
}

void NullDriver::process()
{
    return;
}

void NullDriver::enableEvents()
{
    return;
}

void NullDriver::disableEvents()
{
    return;
}

bool NullDriver::getEvent(InputEvent *)
{
    // There are never events to process with NullDriver.
    return false;
}

}

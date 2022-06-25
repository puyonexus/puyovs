#include "driver.h"
#include "drivers/sdldriver.h"
#include "drivers/dinputdriver.h"
#include "drivers/nulldriver.h"

namespace ilib
{

Driver *inputDriver = 0;

Driver *getDriver()
{
    if(inputDriver) return inputDriver;

    #define driver(class) inputDriver = new class;\
        if(!inputDriver->error()) return inputDriver; else { delete inputDriver; inputDriver = 0; }

#ifdef ILIB_USE_DRIVER_SDL
    driver(SDLDriver);
#endif
#ifdef ILIB_USE_DRIVER_DINPUT
    driver(DInputDriver);
#endif

    driver(NullDriver);

    #undef driver

    return inputDriver;
}

}


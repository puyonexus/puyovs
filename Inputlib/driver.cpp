#include "driver.h"
#include "drivers/dinputdriver.h"
#include "drivers/nulldriver.h"
#include "drivers/sdldriver.h"

namespace ilib {

Driver* inputDriver = nullptr;

Driver* getDriver()
{
	if (inputDriver)
		return inputDriver;

#define driver(class)              \
	do {                           \
		inputDriver = new class;   \
		if (!inputDriver->error()) \
			return inputDriver;    \
		delete inputDriver;        \
		inputDriver = nullptr;     \
	} while (0)

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

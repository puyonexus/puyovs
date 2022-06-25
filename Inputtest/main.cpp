#include <ilib/inputlib.h>
#include <ilib/driver.h>
#ifdef _WIN32
#include <Windows.h>
#else
#include <unistd.h>
#endif
#include <math.h>

int main()
{
    ilib::Driver *driver = ilib::getDriver();

    if(!driver)
    {
        ILIB_ERROR("Unable to get input driver.\n");
        return 1;
    }

    driver->enableEvents();

    while(1)
    {
        ilib::InputEvent e;
        while(driver->getEvent(&e))
        {
            switch(e.type)
            {
            case ilib::InputEvent::ButtonDownEvent:
            case ilib::InputEvent::ButtonUpEvent:
                printf("Button %i %s on controller %i.\n",
                       e.button.id, e.type == ilib::InputEvent::ButtonDownEvent ? "pressed" : "released", e.device);
                break;
            case ilib::InputEvent::AxisEvent:
                if(fabs(e.axis.value) > 0.5)
                printf("Axis %i moved to %f on controller %i.\n",
                       e.axis.id, e.axis.value, e.device);
                break;
            case ilib::InputEvent::HatEvent:
                printf("Hat %i moved to %i on controller %i.\n",
                       e.hat.id, int(e.hat.value), e.device);
            default:
                printf("Unknown event.\n");
            }
        }
        driver->process();
#ifdef _WIN32
        Sleep(1000 / 60);
#else
        usleep(1000000 / 60);
#endif
    }

    return 0;
}


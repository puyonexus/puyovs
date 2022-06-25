#ifdef ILIB_USE_DRIVER_SDL

#include "drivers/sdldriver.h"
#include "inputevent.h"

#include <SDL.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#if !defined(_WIN32) && !defined(__APPLE__)
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#endif
#include <signal.h>
#include <math.h>

#include <vector>
#include <deque>

#ifndef NAN
    #define NAN (1.0 / 0.0)
#endif

namespace ilib {

struct SDLDriver::Priv
{
    struct Gamepad : public Driver::Gamepad
    {
        SDL_Joystick *mJoystick;
        unsigned char mNumButtons, mNumAxes, mNumHats;

        Gamepad(SDL_Joystick *joystick)
            : mJoystick(joystick)
        {
            mNumButtons = SDL_JoystickNumButtons(joystick);
            mNumAxes = SDL_JoystickNumAxes(joystick);
            mNumHats = SDL_JoystickNumHats(joystick);
        }

        ~Gamepad() { }

        int numButtons() { return mNumButtons; }
        int numAxis() { return mNumAxes; }
        int numHats() { return mNumHats; }

        bool button(int num)
        {
            if(num < 0 || num >= mNumButtons) return false;

            return SDL_JoystickGetButton(mJoystick, num) == 1;
        }

        float axis(int num)
        {
            if(num < 0 || num >= mNumAxes) return NAN;

            return SDL_JoystickGetAxis(mJoystick, num) / 32767;
        }

        HatPosition hat(int num)
        {
            if(num < 0 || num >= mNumHats) return HatCentered;

            HatPosition position;

            switch(SDL_JoystickGetHat(mJoystick, num))
            {
            case SDL_HAT_CENTERED : position = HatCentered ; break;
            case SDL_HAT_UP       : position = HatUp       ; break;
            case SDL_HAT_RIGHT    : position = HatRight    ; break;
            case SDL_HAT_DOWN     : position = HatDown     ; break;
            case SDL_HAT_LEFT     : position = HatLeft     ; break;
            case SDL_HAT_RIGHTUP  : position = HatRightUp  ; break;
            case SDL_HAT_RIGHTDOWN: position = HatRightDown; break;
            case SDL_HAT_LEFTUP   : position = HatLeftUp   ; break;
            case SDL_HAT_LEFTDOWN : position = HatLeftDown ; break;
            }

            return position;
        }
    };

    bool error;
    int processEvents;
    std::vector<Gamepad> gamepads;
    std::deque<InputEvent> events;

    bool initSdl()
    {
        int result;
        error = false;

        result = SDL_InitSubSystem(SDL_INIT_GAMECONTROLLER);
        if(result < 0)
        {
            ILIB_ERROR("Error initializing SDL: %s\n", SDL_GetError());

            error = true;
            return false;
        }

        signal(SIGINT, SIG_DFL);

        processEvents = 0;
        SDL_JoystickEventState(SDL_DISABLE);

        int numJoysticks = SDL_NumJoysticks();
        for(int i = 0; i < numJoysticks; i++)
        {
            SDL_Joystick *joy = SDL_JoystickOpen(i);

            // No joy? :(
            if(!joy)
            {
                ILIB_ERROR("Couldn't open joypad %i.\n", i);
                continue;
            } else ILIB_WARN("Opened joypad %s.\n", SDL_JoystickName(joy));

            gamepads.push_back(joy);
        }

        return true;
    }

    void process()
    {
        if(processEvents > 0)
        {
            SDL_Event e;

            SDL_PumpEvents();
            while(SDL_PeepEvents(&e, 1, SDL_GETEVENT, SDL_JOYAXISMOTION, SDL_JOYDEVICEREMOVED) > 0)
            {
                switch(e.type)
                {
                case SDL_JOYBUTTONDOWN:
                    events.push_back(InputEvent::createButtonEvent(InputEvent::ButtonDownEvent, e.jbutton.which, e.jbutton.button));
                    break;
                case SDL_JOYBUTTONUP:
                    events.push_back(InputEvent::createButtonEvent(InputEvent::ButtonUpEvent, e.jbutton.which, e.jbutton.button));
                    break;
                case SDL_JOYAXISMOTION:
                    events.push_back(InputEvent::createAxisEvent(e.jaxis.which, e.jaxis.axis, e.jaxis.value / 32767.f));
                    break;
                case SDL_JOYHATMOTION:
                    HatPosition position;

                    switch(e.jhat.value)
                    {
                    case SDL_HAT_CENTERED: position = HatCentered; break;
                    case SDL_HAT_UP      : position = HatUp      ; break;
                    case SDL_HAT_RIGHT   : position = HatRight   ; break;
                    case SDL_HAT_DOWN    : position = HatDown    ; break;
                    case SDL_HAT_LEFT    : position = HatLeft    ; break;
                    }

                    events.push_back(InputEvent::createHatEvent(e.jhat.which, e.jhat.hat, position));
                    break;
                }
            }
        }
    }

    bool getEvent(InputEvent *e)
    {
        if(!events.empty())
        {
            *e = events.front();
            events.pop_front();

            return true;
        }
        else return false;
    }
};

SDLDriver::SDLDriver()
    : p(new Priv)
{
    if(!p->initSdl()) return;
}

SDLDriver::~SDLDriver()
{
    delete p;
}

bool SDLDriver::error()
{
    return p->error;
}

void SDLDriver::process()
{
    p->process();
}

void SDLDriver::enableEvents()
{
    p->processEvents++;

    if(p->processEvents == 1)
        SDL_JoystickEventState(SDL_ENABLE);
}

void SDLDriver::disableEvents()
{
    p->processEvents--;

    if(p->processEvents == 0)
        SDL_JoystickEventState(SDL_DISABLE);
}

bool SDLDriver::getEvent(InputEvent *e)
{
    return p->getEvent(e);
}

}

#endif

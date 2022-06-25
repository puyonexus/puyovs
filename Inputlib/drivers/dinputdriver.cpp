#ifdef ILIB_USE_DRIVER_DINPUT

#include <windows.h>

#define DIRECTINPUT_VERSION 0x0800
#include "initguid.h"
#include <dinput.h>

#include "dinputdriver.h"
#include "inputevent.h"

#include <vector>
#include <deque>

namespace ilib {

typedef HRESULT(WINAPI * DIRECTINPUT8CREATE)(HINSTANCE hinst, DWORD dwVersion, REFIID riidltf, LPVOID *ppvOut, LPUNKNOWN punkOuter);
#include "js2format.h"

struct DInputDriver::Priv
{
    bool error;
    int processEvents;
    std::deque<InputEvent> events;

    struct DInput
    {
        HINSTANCE module;
        DIRECTINPUT8CREATE create;
        LPDIRECTINPUT8 context;
        LPDIRECTINPUTDEVICE8 device;

        struct Gamepad : public Driver::Gamepad
        {
            LPDIRECTINPUTDEVICE8 handle;

            short hats[4];
            short axes[6];
            bool buttons[128];
            int id;

            Gamepad(LPDIRECTINPUTDEVICE8 handle, int id)
                : handle(handle), id(id)
            {
                for(int i = 0; i < 4; ++i) hats[i] = HatCentered;
                for(int i = 0; i < 6; ++i) axes[i] = 0;
                for(int i = 0; i < 128; ++i) buttons[i] = false;
            }

            void update(DIJOYSTATE2 &state, Priv *p)
            {
                bool events = p->processEvents > 0;

                for(int i = 0; i < 4; ++i)
                {
                    unsigned hatBefore = hats[i];
                    hats[i] = HatCentered;

                    unsigned pov = state.rgdwPOV[i];
                    if(pov >= 36000);
                    else
                    {
                        if(pov >= 31500 || pov <=  4500) hats[i] |= HatUp;
                        if(pov >=  4500 && pov <= 13500) hats[i] |= HatRight;
                        if(pov >= 13500 && pov <= 22500) hats[i] |= HatDown;
                        if(pov >= 22500 && pov <= 31500) hats[i] |= HatLeft;
                    }

                    if(events && hats[i] != hatBefore)
                        p->queue(InputEvent::createHatEvent(id, i, HatPosition(hats[i])));
                }

                #define axis(i, member) \
                    if(events && axes[i] != state . member) \
                        p->queue(InputEvent::createAxisEvent(id, i, state . member /  32767.f)); \
                    axes[i] = state . member

                axis(0, lX);
                axis(1, lY);
                axis(2, lZ);
                axis(3, lRx);
                axis(4, lRy);
                axis(5, lRz);

                #undef axis

                for(int i = 0; i < 128; ++i)
                {
                    if(events && buttons[i] != (bool)state.rgbButtons[i])
                    {
                        InputEvent::Type type = state.rgbButtons[i] ? InputEvent::ButtonDownEvent : InputEvent::ButtonUpEvent;
                        p->queue(InputEvent::createButtonEvent(type, id, i));
                    }

                    buttons[i] = (bool)state.rgbButtons[i];
                }
            }

            // TODO: proper implementation
            int numButtons() { return 128; }
            int numAxis() { return 6; }
            int numHats() { return 4; }

            bool button(int num)
            {
                if(num < 0 || num >= 128) return false;

                return buttons[num];
            }

            float axis(int num)
            {
                if(num < 0 || num >= 6) return 0.f;

                return axes[num] / 32767.f;
            }

            HatPosition hat(int num)
            {
                if(num < 0 || num >= 4) return HatCentered;

                return HatPosition(hats[num]);
            }
        };
        std::vector<Gamepad> gamepads;

        static BOOL CALLBACK enum_joypads(const DIDEVICEINSTANCE *i, void *p)
        {
            return ((DInput*)p)->init_joypad(i);
        }

        bool init_joypad(const DIDEVICEINSTANCE *instance)
        {
            if(FAILED(context->CreateDevice(instance->guidInstance, &device, 0)))
                return DIENUM_CONTINUE;

            device->SetDataFormat(&js2format);
            device->SetCooperativeLevel(GetDesktopWindow(), DISCL_NONEXCLUSIVE | DISCL_BACKGROUND);
            device->EnumObjects(enum_axes, (void*)this, DIDFT_ABSAXIS);
            gamepads.push_back(Gamepad(device, gamepads.size())); // TODO: better id

            return DIENUM_CONTINUE;
        }

        static BOOL CALLBACK enum_axes(const DIDEVICEOBJECTINSTANCE *i, void *p)
        {
            return ((DInput*)p)->init_axis(i);
        }

        bool init_axis(const DIDEVICEOBJECTINSTANCE *instance)
        {
            DIPROPRANGE range;
            range.diph.dwSize = sizeof(DIPROPRANGE);
            range.diph.dwHeaderSize = sizeof(DIPROPHEADER);
            range.diph.dwHow = DIPH_BYID;
            range.diph.dwObj = instance->dwType;
            range.lMin = -32768;
            range.lMax = 32767;
            device->SetProperty(DIPROP_RANGE, &range.diph);
            return DIENUM_CONTINUE;
        }

        bool load()
        {
            module = LoadLibraryW(L"DINPUT8.DLL");
            if(!module) return false;

            create = (DIRECTINPUT8CREATE) GetProcAddress(module, "DirectInput8Create");
            if(!create) return false;

            create(GetModuleHandle(0), DIRECTINPUT_VERSION, IID_IDirectInput8, (void**)&context, 0);
            context->EnumDevices(DI8DEVCLASS_GAMECTRL, enum_joypads, (void*)this, DIEDFL_ATTACHEDONLY);

            return true;
        }

        void unload()
        {
            create = 0;

            FreeLibrary(module);
            module = 0;
        }

        void process(Priv *p)
        {
            for(unsigned i = 0; i < gamepads.size(); i++)
            {
                if(FAILED(gamepads[i].handle->Poll()))
                {
                    gamepads[i].handle->Acquire();
                    continue;
                }

                DIJOYSTATE2 state;
                gamepads[i].handle->GetDeviceState(sizeof(DIJOYSTATE2), &state);
                gamepads[i].update(state, p);
            }
        }
    } dinput;

    bool init()
    {
        error = false;
        processEvents = 0;

        if(!dinput.load())
            return false;

        return true;
    }

    void process()
    {
        dinput.process(this);
    }

    void queue(const InputEvent &e)
    {
        events.push_back(e);
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


DInputDriver::DInputDriver()
    : p(new Priv)
{
    p->init();
}

DInputDriver::~DInputDriver()
{
    delete p;
}

bool DInputDriver::error()
{
    return p->error;
}

void DInputDriver::process()
{
    p->process();
}

void DInputDriver::enableEvents()
{
    p->processEvents++;
}

void DInputDriver::disableEvents()
{
    p->processEvents--;
}

bool DInputDriver::getEvent(InputEvent *e)
{
    return p->getEvent(e);
}

}

#endif

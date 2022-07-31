#ifdef ILIB_USE_DRIVER_DINPUT

#include <windows.h>

#define DIRECTINPUT_VERSION 0x0800
#include "initguid.h"
#include <dinput.h>

#include "dinputdriver.h"
#include "inputevent.h"

#include <deque>
#include <vector>

namespace ilib {

typedef HRESULT(WINAPI* DIRECTINPUT8CREATE)(HINSTANCE hinst, DWORD dwVersion, REFIID riidltf, LPVOID* ppvOut, LPUNKNOWN punkOuter);
#include "js2format.h"

struct DInputDriver::Priv {
	bool error;
	int processEvents;
	std::deque<InputEvent> events;

	struct DInput {
		HINSTANCE module;
		DIRECTINPUT8CREATE create;
		LPDIRECTINPUT8 context;
		LPDIRECTINPUTDEVICE8 device;

		struct Gamepad : Driver::Gamepad {
			LPDIRECTINPUTDEVICE8 handle;

			HatPosition hats[4] = { HatPosition::HatCentered };
			short axes[6] = { 0 };
			bool buttons[128] = { false };
			int id;

			Gamepad(LPDIRECTINPUTDEVICE8 handle, int id)
				: handle(handle)
				, id(id)
			{
			}

			void update(DIJOYSTATE2& state, Priv* p)
			{
				const bool events = p->processEvents > 0;

				for (int i = 0; i < 4; ++i) {
					const HatPosition hatBefore = hats[i];
					hats[i] = HatPosition::HatCentered;

					const unsigned pov = state.rgdwPOV[i];
					if (pov < 36000) {
						if (pov >= 31500 || pov <= 4500)
							hats[i] |= HatPosition::HatUp;
						if (pov >= 4500 && pov <= 13500)
							hats[i] |= HatPosition::HatRight;
						if (pov >= 13500 && pov <= 22500)
							hats[i] |= HatPosition::HatDown;
						if (pov >= 22500 && pov <= 31500)
							hats[i] |= HatPosition::HatLeft;
					}

					if (events && hats[i] != hatBefore) {
						p->queue(InputEvent::createHatEvent(id, i, static_cast<HatPosition>(hats[i])));
					}
				}

#define AXIS(i, member)                                                       \
	if (events && axes[i] != state.member)                                    \
		p->queue(InputEvent::createAxisEvent(id, i, state.member / 32767.f)); \
	axes[i] = static_cast<short>(state.member)
				AXIS(0, lX);
				AXIS(1, lY);
				AXIS(2, lZ);
				AXIS(3, lRx);
				AXIS(4, lRy);
				AXIS(5, lRz);
#undef AXIS

				for (int i = 0; i < 128; ++i) {
					if (events && buttons[i] != static_cast<bool>(state.rgbButtons[i])) {
						const InputEvent::Type type = state.rgbButtons[i] ? InputEvent::Type::ButtonDownEvent : InputEvent::Type::ButtonUpEvent;
						p->queue(InputEvent::createButtonEvent(type, id, i));
					}

					buttons[i] = static_cast<bool>(state.rgbButtons[i]);
				}
			}

			// TODO: proper implementation
			[[nodiscard]] int numButtons() const override { return 128; }
			[[nodiscard]] int numAxis() const override { return 6; }
			[[nodiscard]] int numHats() const override { return 4; }

			[[nodiscard]] bool button(int num) const override
			{
				if (num < 0 || num >= 128)
					return false;

				return buttons[num];
			}

			[[nodiscard]] float axis(int num) const override
			{
				if (num < 0 || num >= 6)
					return 0.f;

				return axes[num] / 32767.f;
			}

			[[nodiscard]] HatPosition hat(int num) const override
			{
				if (num < 0 || num >= 4)
					return HatPosition::HatCentered;

				return static_cast<HatPosition>(hats[num]);
			}
		};
		std::vector<Gamepad> gamepads;

		static BOOL CALLBACK enum_joypads(const DIDEVICEINSTANCE* i, void* p)
		{
			return static_cast<DInput*>(p)->init_joypad(i);
		}

		bool init_joypad(const DIDEVICEINSTANCE* instance)
		{
			if (FAILED(context->CreateDevice(instance->guidInstance, &device, nullptr))) {
				return DIENUM_CONTINUE;
			}

			device->SetDataFormat(&js2format);
			device->SetCooperativeLevel(GetDesktopWindow(), DISCL_NONEXCLUSIVE | DISCL_BACKGROUND);
			device->EnumObjects(enum_axes, (void*)this, DIDFT_ABSAXIS);
			gamepads.push_back(Gamepad(device, static_cast<int>(gamepads.size()))); // TODO: better id

			return DIENUM_CONTINUE;
		}

		static BOOL CALLBACK enum_axes(const DIDEVICEOBJECTINSTANCE* i, void* p)
		{
			return static_cast<DInput*>(p)->initAxis(i);
		}

		bool initAxis(const DIDEVICEOBJECTINSTANCE* instance) const
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
			if (!module)
				return false;

			create = reinterpret_cast<DIRECTINPUT8CREATE>(GetProcAddress(module, "DirectInput8Create"));
			if (!create)
				return false;

			create(GetModuleHandle(nullptr), DIRECTINPUT_VERSION, IID_IDirectInput8, reinterpret_cast<void**>(&context), nullptr);
			context->EnumDevices(DI8DEVCLASS_GAMECTRL, enum_joypads, (void*)this, DIEDFL_ATTACHEDONLY);

			return true;
		}

		void unload()
		{
			create = nullptr;

			FreeLibrary(module);
			module = nullptr;
		}

		void process(Priv* p)
		{
			for (auto& gamepad : gamepads) {
				if (FAILED(gamepad.handle->Poll())) {
					gamepad.handle->Acquire();
					continue;
				}

				DIJOYSTATE2 state;
				gamepad.handle->GetDeviceState(sizeof(DIJOYSTATE2), &state);
				gamepad.update(state, p);
			}
		}
	} dinput;

	bool init()
	{
		error = false;
		processEvents = 0;

		return dinput.load();
	}

	void process()
	{
		dinput.process(this);
	}

	void queue(const InputEvent& e)
	{
		events.push_back(e);
	}

	bool getEvent(InputEvent* e)
	{
		if (events.empty()) {
			return false;
		}

		*e = events.front();
		events.pop_front();

		return true;
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

bool DInputDriver::error() const
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

bool DInputDriver::getEvent(InputEvent* e)
{
	return p->getEvent(e);
}

}

#endif

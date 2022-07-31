#ifdef ILIB_USE_DRIVER_SDL

#include "drivers/sdldriver.h"
#include "inputevent.h"

#include <SDL.h>
#include <math.h>
#include <signal.h>

#include <deque>
#include <vector>

namespace ilib {

struct SDLDriver::Priv {
	struct Gamepad final : Driver::Gamepad {
		SDL_Joystick* mJoystick;
		unsigned char mNumButtons, mNumAxes, mNumHats;

		Gamepad(SDL_Joystick* joystick)
			: mJoystick(joystick)
		{
			mNumButtons = SDL_JoystickNumButtons(joystick);
			mNumAxes = SDL_JoystickNumAxes(joystick);
			mNumHats = SDL_JoystickNumHats(joystick);
		}

		~Gamepad() override = default;

		[[nodiscard]] int numButtons() const override { return mNumButtons; }
		[[nodiscard]] int numAxis() const override { return mNumAxes; }
		[[nodiscard]] int numHats() const override { return mNumHats; }

		[[nodiscard]] bool button(int num) const override
		{
			if (num < 0 || num >= mNumButtons) {
				return false;
			}

			return SDL_JoystickGetButton(mJoystick, num) == 1;
		}

		[[nodiscard]] float axis(int num) const override
		{
			if (num < 0 || num >= mNumAxes) {
				return nanf("");
			}

			return SDL_JoystickGetAxis(mJoystick, num) / 32767.f;
		}

		[[nodiscard]] HatPosition hat(int num) const override
		{
			if (num < 0 || num >= mNumHats) {
				return HatPosition::HatCentered;
			}

			switch (SDL_JoystickGetHat(mJoystick, num)) {
			default:
			case SDL_HAT_CENTERED:
				return HatPosition::HatCentered;
			case SDL_HAT_UP:
				return HatPosition::HatUp;
			case SDL_HAT_RIGHT:
				return HatPosition::HatRight;
			case SDL_HAT_DOWN:
				return HatPosition::HatDown;
			case SDL_HAT_LEFT:
				return HatPosition::HatLeft;
			case SDL_HAT_RIGHTUP:
				return HatPosition::HatRightUp;
			case SDL_HAT_RIGHTDOWN:
				return HatPosition::HatRightDown;
			case SDL_HAT_LEFTUP:
				return HatPosition::HatLeftUp;
			case SDL_HAT_LEFTDOWN:
				return HatPosition::HatLeftDown;
			}
		}
	};

	bool error = false;
	int processEvents = 0;
	std::vector<Gamepad> gamepads;
	std::deque<InputEvent> events;

	bool initSdl()
	{
		error = false;

		if (const int result = SDL_InitSubSystem(SDL_INIT_GAMECONTROLLER); result < 0) {
			ILIB_ERROR("Error initializing SDL: %s\n", SDL_GetError());

			error = true;
			return false;
		}

		signal(SIGINT, SIG_DFL);

		processEvents = 0;
		SDL_JoystickEventState(SDL_DISABLE);

		const int numJoysticks = SDL_NumJoysticks();
		for (int i = 0; i < numJoysticks; i++) {
			SDL_Joystick* joy = SDL_JoystickOpen(i);

			// No joy? :(
			if (!joy) {
				ILIB_ERROR("Couldn't open joypad %i.\n", i);
				continue;
			}

			ILIB_WARN("Opened joypad %s.\n", SDL_JoystickName(joy));

			gamepads.emplace_back(joy);
		}

		return true;
	}

	void process()
	{
		if (processEvents <= 0) {
			return;
		}

		SDL_Event e;

		SDL_PumpEvents();
		while (SDL_PeepEvents(&e, 1, SDL_GETEVENT, SDL_JOYAXISMOTION, SDL_JOYDEVICEREMOVED) > 0) {
			switch (e.type) {
			case SDL_JOYBUTTONDOWN:
				events.push_back(InputEvent::createButtonEvent(InputEvent::Type::ButtonDownEvent, e.jbutton.which, e.jbutton.button));
				break;

			case SDL_JOYBUTTONUP:
				events.push_back(InputEvent::createButtonEvent(InputEvent::Type::ButtonUpEvent, e.jbutton.which, e.jbutton.button));
				break;

			case SDL_JOYAXISMOTION:
				events.push_back(InputEvent::createAxisEvent(e.jaxis.which, e.jaxis.axis, e.jaxis.value / 32767.f));
				break;

			case SDL_JOYHATMOTION:
				HatPosition position;

				switch (e.jhat.value) {
				default:
				case SDL_HAT_CENTERED:
					position = HatPosition::HatCentered;
					break;
				case SDL_HAT_UP:
					position = HatPosition::HatUp;
					break;
				case SDL_HAT_RIGHT:
					position = HatPosition::HatRight;
					break;
				case SDL_HAT_DOWN:
					position = HatPosition::HatDown;
					break;
				case SDL_HAT_LEFT:
					position = HatPosition::HatLeft;
					break;
				case SDL_HAT_RIGHTUP:
					position = HatPosition::HatRightUp;
					break;
				case SDL_HAT_RIGHTDOWN:
					position = HatPosition::HatRightDown;
					break;
				case SDL_HAT_LEFTUP:
					position = HatPosition::HatLeftUp;
					break;
				case SDL_HAT_LEFTDOWN:
					position = HatPosition::HatLeftDown;
					break;
				}

				events.push_back(InputEvent::createHatEvent(e.jhat.which, e.jhat.hat, position));
				break;

			default:
				break;
			}
		}
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

SDLDriver::SDLDriver()
	: p(new Priv)
{
	if (!p->initSdl()) {
		return;
	}
}

SDLDriver::~SDLDriver()
{
	delete p;
}

bool SDLDriver::error() const
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

	if (p->processEvents != 1) {
		return;
	}

	SDL_JoystickEventState(SDL_ENABLE);
}

void SDLDriver::disableEvents()
{
	p->processEvents--;

	if (p->processEvents != 0) {
		return;
	}

	SDL_JoystickEventState(SDL_DISABLE);
}

bool SDLDriver::getEvent(InputEvent* e)
{
	return p->getEvent(e);
}

}

#endif

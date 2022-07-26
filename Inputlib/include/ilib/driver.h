#pragma once

#include "inputevent.h"
#include "inputlib.h"

namespace ilib {

class Driver;
Driver* getDriver();

class Driver : NonCopyable {
public:
	struct Gamepad {
		virtual ~Gamepad() { }

		[[nodiscard]] virtual int numButtons() const = 0;
		[[nodiscard]] virtual int numAxis() const = 0;
		[[nodiscard]] virtual int numHats() const = 0;

		[[nodiscard]] virtual bool button(int num) const = 0;
		[[nodiscard]] virtual float axis(int num) const = 0;
		[[nodiscard]] virtual HatPosition hat(int num) const = 0;
	};

	virtual ~Driver() {};

	virtual void process() = 0;
	virtual bool error() const = 0;

	virtual void enableEvents() = 0;
	virtual void disableEvents() = 0;
	virtual bool getEvent(InputEvent* e) = 0;
};

}

#pragma once

#include "inputlib.h"

namespace ilib {

struct InputEvent {
	enum class Type {
	    ButtonDownEvent,
		ButtonUpEvent,
		AxisEvent,
		HatEvent
	};

	Type type;
	int device;

	union {
		struct
		{
			int id;
		} button;

		struct
		{
			int id;
			float value;
		} axis;

		struct
		{
			int id;
			HatPosition value;
		} hat;
	};

	static InputEvent createButtonEvent(Type type, int device, int button)
	{
		InputEvent e;

		e.type = type;
		e.device = device;
		e.button.id = button;

		return e;
	}

	static InputEvent createAxisEvent(int device, int axis, float value)
	{
		InputEvent e;

		e.type = Type::AxisEvent;
		e.device = device;
		e.axis.id = axis;
		e.axis.value = value;

		return e;
	}

	static InputEvent createHatEvent(int device, int hat, HatPosition value)
	{
		InputEvent e;

		e.type = Type::HatEvent;
		e.device = device;
		e.hat.id = hat;
		e.hat.value = value;

		return e;
	}
};

}

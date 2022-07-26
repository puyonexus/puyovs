#pragma once

#include "common.h"
#include <string>

namespace alib {

class Stream;
class Device;
extern Device* device;

Device* open();
void close();

class Device : NonCopyable {
	ALIB_DECLARE_PRIV;

public:
	Device();
	~Device();

	bool play(const Stream& stm);
	void setVolume(float volume);
	void setSoundVolume(float volume);
	void setMusicVolume(float volume);
};

}

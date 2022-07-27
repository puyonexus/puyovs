#pragma once

#include "audiolib.h"

namespace alib {

class RingBuffer {
	ALIB_DECLARE_PRIV;

public:
	RingBuffer(unsigned int bufferSize);
	~RingBuffer();

	int write(const void* data, unsigned int len);
	int read(void* outData, unsigned int len);
	int seek(unsigned int len);

	[[nodiscard]] unsigned int freeSpace() const;
	[[nodiscard]] unsigned int length() const;

	[[nodiscard]] unsigned int readPtr() const;
	[[nodiscard]] unsigned int writePtr() const;
};

}

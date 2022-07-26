#pragma once

#include <stdint.h>

// Known SP values
const uint32_t CLASSICRNG_SP_INIT = 0x00FF7FE8;
const uint32_t CLASSICRNG_SP_COLORSWAP = 0x00FF7FE8;
const uint32_t CLASSICRNG_SP_GENERATION = 0x00FF7FF4;

class ClassicRng {
public:
	explicit ClassicRng(const uint32_t value)
		: m_value(value)
	{
	}

	uint32_t m_value;

	void next(uint32_t* d0, uint32_t sp);
};

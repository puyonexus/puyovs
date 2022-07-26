#pragma once

#include "PuyoRng.h"

class ClassicRng;

class ClassicPool {
public:
	explicit ClassicPool(ClassicRng* rng);

	Piece m_pool[3][256] {};
};

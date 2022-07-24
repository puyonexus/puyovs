#pragma once

#include "PuyoRNG.h"

class ClassicRNG;

class ClassicPool {
public:
	explicit ClassicPool(ClassicRNG* rng);

	Piece pool[3][256];
};

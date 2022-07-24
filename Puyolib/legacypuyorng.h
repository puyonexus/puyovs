#pragma once

#include "puyorng.h"

class MersenneTwister;

class LegacyPuyoRNG : public PuyoRNG {
public:
	LegacyPuyoRNG(uint32_t seed, int index);

	Piece next(int colors);

private:
	MersenneTwister* mt;
	int index;
};

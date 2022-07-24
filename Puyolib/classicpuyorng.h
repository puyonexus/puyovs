#pragma once

#include "classicpool.h"
#include "puyorng.h"

class ClassicPuyoRNG : public PuyoRNG {
public:
	explicit ClassicPuyoRNG(uint32_t seed = 0, int index = 0);

	Piece next(int colors);

private:
	ClassicPool pool;
	int index;
};

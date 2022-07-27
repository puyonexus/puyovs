#pragma once

#include "PuyoRng.h"

class MersenneTwister;

class LegacyPuyoRng final : public PuyoRng {
public:
	LegacyPuyoRng(uint32_t seed, int index);

	Piece next(int colors) override;

private:
	MersenneTwister* m_mt;
	int m_index{};
};

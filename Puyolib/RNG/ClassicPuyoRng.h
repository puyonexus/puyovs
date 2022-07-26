#pragma once

#include "ClassicPool.h"
#include "PuyoRng.h"

class ClassicPuyoRng final : public PuyoRng {
public:
	explicit ClassicPuyoRng(uint32_t seed = 0, int index = 0);

	Piece next(int colors) override;

private:
	ClassicPool m_pool;
	int m_index;
};

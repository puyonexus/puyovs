#include "LegacyPuyoRng.h"
#include "MersenneTwister.h"

LegacyPuyoRng::LegacyPuyoRng(const uint32_t seed, const int index)
	: m_mt(new MersenneTwister)
{
	m_mt->init_genrand(seed);
	for (int i = 0; i < index; i++) {
        LegacyPuyoRng::next(4);
	}
}

Piece LegacyPuyoRng::next(int colors)
{
	switch (static_cast<int>(m_mt->random() * colors)) {
	default:
	case 0:
		return Red;
	case 1:
		return Green;
	case 2:
		return Blue;
	case 3:
		return Yellow;
	case 4:
		return Purple;
	}
}

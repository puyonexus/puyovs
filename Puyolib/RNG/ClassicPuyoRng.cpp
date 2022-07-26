#include "ClassicPuyoRng.h"
#include "ClassicRNG.h"

ClassicPuyoRng::ClassicPuyoRng(const uint32_t seed, const int index)
	: m_pool(new ClassicRng(seed))
	, m_index(index)
{
}

Piece ClassicPuyoRng::next(const int colors)
{
	switch (colors) {
	case 3:
		return m_pool.m_pool[0][(m_index++) & 0xFF];
	default:
	case 4:
		return m_pool.m_pool[1][(m_index++) & 0xFF];
	case 5:
		return m_pool.m_pool[2][(m_index++) & 0xFF];
	}
}

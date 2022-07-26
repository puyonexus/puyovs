#include "ClassicPool.h"
#include "ClassicRNG.h"
#include "PuyoRng.h"

ClassicPool::ClassicPool(ClassicRng* rng)
{
	constexpr Piece colorset[5] = {
		Red,
		Yellow,
		Green,
		Blue,
		Purple
	};

	// Shuffle the colorset.
	for (int i = 0; i < 5; i++) {
		uint32_t v = 4;
		rng->next(&v, CLASSICRNG_SP_COLORSWAP);
		uint32_t x = (v & 0xffff) * 4 >> 16;
	}

	// Fill the pools.
	for (int c = 0; c < 3; c++) {
		for (int i = 0xff; i >= 0; i--) {
			m_pool[c][i] = colorset[i % (c + 3)];
		}
	}

	// Shuffle (3 color)
	uint32_t v = 0x23640000;
	for (int i = 0xff; i >= 0; i--) {
		rng->next(&v, CLASSICRNG_SP_GENERATION);
		uint32_t x = v & 0xff;
		std::swap(m_pool[1][i], m_pool[1][x]);
		v &= 0xffff00ff;
	}

	v |= 0xffff;

	// Shuffle (5 color)
	for (int i = 0xff; i >= 0; i--) {
		rng->next(&v, CLASSICRNG_SP_GENERATION);
		uint32_t x = v & 0xff;
		std::swap(m_pool[0][i], m_pool[0][x]);
		std::swap(m_pool[2][i], m_pool[2][x]);
		v &= 0xffff00ff;
	}

	// Clobber first 2 pairs in 4, 5 color.
	for (int i = 0; i < 4; i++) {
		m_pool[1][i] = m_pool[0][i];
		m_pool[2][i] = m_pool[0][i];
	}

	// Clobber next 2 pairs in 5 color.
	for (int i = 4; i < 8; i++) {
		m_pool[2][i] = m_pool[0][i];
	}
}

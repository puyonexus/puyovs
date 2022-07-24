#include "ClassicPuyoRNG.h"
#include "ClassicRNG.h"

ClassicPuyoRNG::ClassicPuyoRNG(uint32_t seed, int index)
	: pool(new ClassicRNG(seed)), index(index) {
}

Piece ClassicPuyoRNG::next(int colors) {
	switch (colors) {
	case 3:
		return pool.pool[0][(index++) & 0xFF];
	default:
	case 4:
		return pool.pool[1][(index++) & 0xFF];
	case 5:
		return pool.pool[2][(index++) & 0xFF];
	}
}

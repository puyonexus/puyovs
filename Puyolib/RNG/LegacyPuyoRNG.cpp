#include "LegacyPuyoRNG.h"
#include "MersenneTwister.h"

LegacyPuyoRNG::LegacyPuyoRNG(uint32_t seed, int index)
	: mt(new MersenneTwister) {
	mt->init_genrand(seed);
	for (int i = 0; i < index; i++) {
		next(4);
	}
}

Piece LegacyPuyoRNG::next(int colors) {
	switch (int(mt->random() * colors)) {
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

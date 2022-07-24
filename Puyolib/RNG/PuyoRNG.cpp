#include "PuyoRNG.h"
#include "ClassicPuyoRNG.h"
#include "LegacyPuyoRNG.h"

PuyoRNG* MakePuyoRNG(const std::string& name, uint32_t seed, int index) {
	if (name == "classic") {
		return new ClassicPuyoRNG(seed, index);
	}
	if (name == "legacy") {
		return new LegacyPuyoRNG(seed, index);
	}
	return nullptr;
}

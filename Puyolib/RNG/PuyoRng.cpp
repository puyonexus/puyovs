#include "PuyoRng.h"
#include "ClassicPuyoRng.h"
#include "LegacyPuyoRng.h"

PuyoRng* makePuyoRng(const std::string& name, const uint32_t seed, const int index) {
	if (name == "classic") {
		return new ClassicPuyoRng(seed, index);
	}
	if (name == "legacy") {
		return new LegacyPuyoRng(seed, index);
	}
	return nullptr;
}

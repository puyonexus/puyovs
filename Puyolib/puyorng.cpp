#include "puyorng.h"
#include "classicpuyorng.h"
#include "legacypuyorng.h"


PuyoRNG *MakePuyoRNG(const std::string &name, uint32_t seed, int index) {
    if (name == "classic") {
        return new ClassicPuyoRNG(seed, index);
    } else if (name == "legacy") {
        return new LegacyPuyoRNG(seed, index);
    } else {
        return nullptr;
    }
}

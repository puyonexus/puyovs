#pragma once

#include <string>
#include <stdint.h>

// NOTE: This can be replaced with anything provided
// the integer values are under one byte. You may want
// to order it like modern Puyo, RGBYP, which is fine.
// All that matters is that the names exist.
enum Piece {
    Red = 0,
    Green = 1,
    Blue = 2,
    Yellow = 3,
    Purple = 4,
};


class PuyoRNG {
public:
    virtual Piece next(int colors) = 0;
};

PuyoRNG *MakePuyoRNG(const std::string &name, uint32_t seed, int index);

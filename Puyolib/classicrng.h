#pragma once

#include <stdint.h>

// Known SP values
const uint32_t CLASSICRNG_SP_INIT = 0x00FF7FE8;
const uint32_t CLASSICRNG_SP_COLORSWAP = 0x00FF7FE8;
const uint32_t CLASSICRNG_SP_GENERATION = 0x00FF7FF4;


class ClassicRNG {
public:
    explicit ClassicRNG(uint32_t value) : value(value) {}

    uint32_t value;

    void next(uint32_t *d0, uint32_t sp);
};

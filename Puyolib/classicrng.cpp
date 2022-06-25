#include "classicrng.h"


void ClassicRNG::next(uint32_t *d0, uint32_t sp) {
    uint32_t temp = this->value;
    this->value += *d0;
    this->value += sp;
    this->value = (this->value << 5) | (this->value >> 27);
    this->value += temp;
    this->value++;
    *d0 = this->value;
}

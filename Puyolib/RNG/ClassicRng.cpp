#include "ClassicRNG.h"

void ClassicRng::next(uint32_t* d0, const uint32_t sp)
{
	uint32_t temp = this->m_value;
	this->m_value += *d0;
	this->m_value += sp;
	this->m_value = (this->m_value << 5) | (this->m_value >> 27);
	this->m_value += temp;
	this->m_value++;
	*d0 = this->m_value;
}

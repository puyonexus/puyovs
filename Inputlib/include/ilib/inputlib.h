#pragma once

#include <stdio.h>

#define ILIB_DECLARE_PRIV \
	struct Priv;          \
	Priv* p
#define ILIB_WARN(...) fprintf(stderr, __VA_ARGS__)
#define ILIB_ERROR(...) fprintf(stderr, __VA_ARGS__)

namespace ilib {

// Not sure where to put this
enum class HatPosition : int {
	HatCentered = 0,
	HatUp = 1,
	HatRight = 2,
	HatDown = 4,
	HatLeft = 8,
	HatRightUp = HatRight | HatUp,
	HatRightDown = HatRight | HatDown,
	HatLeftUp = HatLeft | HatUp,
	HatLeftDown = HatLeft | HatDown
};

inline HatPosition operator|=(HatPosition& lhs, HatPosition rhs)
{
	lhs = static_cast<HatPosition>(static_cast<int>(lhs) | static_cast<int>(rhs));
	return lhs;
}

inline bool operator&(HatPosition lhs, HatPosition rhs)
{
	return (static_cast<int>(lhs) & static_cast<int>(rhs)) != 0;
}

class NonCopyable {
protected:
	NonCopyable() = default;
	virtual ~NonCopyable() = default;

public:
	NonCopyable(const NonCopyable&) = delete;
	NonCopyable& operator=(const NonCopyable&) = delete;
	NonCopyable(NonCopyable&&) = delete;
	NonCopyable& operator=(NonCopyable&&) = delete;
};

}

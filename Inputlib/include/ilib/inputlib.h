#ifndef INPUTLIB_H
#define INPUTLIB_H

#include <stdio.h>

#define ILIB_DECLARE_PRIV struct Priv; Priv *p
#define ILIB_WARN(...) fprintf(stderr, __VA_ARGS__)
#define ILIB_ERROR(...) fprintf(stderr, __VA_ARGS__)

namespace ilib {

// not sure where to put this
enum HatPosition
{
    HatCentered = 0, HatUp = 1, HatRight = 2, HatDown = 4, HatLeft = 8,
    HatRightUp = HatRight|HatUp, HatRightDown = HatRight|HatDown,
    HatLeftUp = HatLeft|HatUp, HatLeftDown = HatLeft|HatDown
};

class NonCopyable
{
  protected:
    NonCopyable() { }
    ~NonCopyable() { }
  private:
    NonCopyable(const NonCopyable &);
    NonCopyable &operator =(const NonCopyable &);
};

}

#endif // INPUTLIB_H

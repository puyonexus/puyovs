#include "Paths.h"

namespace PuyoVS::ClientNG {

#ifndef PREFIX
#if defined(_WIN32)
#define PREFIX "C:\\PuyoVS"
#elif defined(__APPLE__) && defined(__aarch64__)
#define PREFIX "/opt/homebrew"
#else
#define PREFIX "/usr/local"
#endif
#endif

#if defined(_WIN32)
const char* defaultAssetPath = PREFIX;
#else
const char* defaultAssetPath = PREFIX "/share/puyovs";
#endif

}

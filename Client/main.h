#pragma once
#include <qglobal.h>

#ifndef PREFIX
#if defined(Q_OS_WIN)
#define PREFIX "C:\\Program Files"
#elif defined(Q_OS_MAC) && defined(__aarch64__)
#define PREFIX "/opt/homebrew"
#else
#define PREFIX "/usr/local"
#endif
#endif

#if defined(Q_OS_WIN)
static const char* defaultAssetPath = PREFIX "\\PuyoVS";
#else
static const char* defaultAssetPath = PREFIX "/share/puyovs";
#endif

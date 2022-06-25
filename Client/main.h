#pragma once
#include <qglobal.h>

#ifndef PREFIX
#   if defined(Q_OS_WIN)
#       define PREFIX "C:\\Program Files\\PuyoVS"
#   elif defined(Q_OS_MAC) && defined(__x86_64__)
#       define PREFIX "/usr/local"
#   elif defined(Q_OS_MAC) && defined(__aarch64__)
#       define PREFIX "/opt/homebrew"
#   else
#       define PREFIX "/usr"
#   endif
#endif

#if defined(Q_OS_WIN)
    static const char *defaultAssetPath = PREFIX;
#else
    static const char *defaultAssetPath = PREFIX "/share/puyovs";
#endif

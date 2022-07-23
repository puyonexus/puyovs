QT       -= core gui

TARGET = alib
TEMPLATE = lib
CONFIG += staticlib link_pkgconfig

# For configuration
include(../SDL/SDL.pri)
include(Audiolib-conf.pri)

SOURCES += audiolib.cpp \
    minimp3.c \
    stream.cpp \
    resampler.cpp \
    mixer.cpp \
    buffer.cpp \
    samplereader.cpp \
    readers/vorbisreader.cpp \
    speex/resample.c \
    ringbuffer.cpp \
    readers/vgmreader.cpp \
    readers/pcmreader.cpp \
    tinythread.cpp \
    minivorbis.cpp

HEADERS += \
    fast_mutex.h \
    minivorbis.h \
    minimp3.h \
    samplereader.h \
    tinythread.h \
    readers/vorbisreader.h \
    speex/stack_alloc.h \
    speex/speex_types.h \
    speex/speex_resampler.h \
    speex/fixed_generic.h \
    speex/fixed_debug.h \
    speex/fixed_bfin.h \
    speex/fixed_arm5e.h \
    speex/fixed_arm4.h \
    speex/arch.h \
    include/alib/stream.h \
    include/alib/sample.h \
    include/alib/audiolib.h \
    include/alib/resampler.h \
    include/alib/mixer.h \
    include/alib/buffer.h \
    include/alib/ringbuffer.h \
    readers/vgmreader.h \
    readers/pcmreader.h \
    include/alib/common.h

OTHER_FILES += \
    COPYRIGHT \
    LICENSE.lgpl \
    LICENSE.isc \
    Audiolib.pri

QMAKE_CXXFLAGS+=-fno-inline

INCLUDEPATH += include/alib

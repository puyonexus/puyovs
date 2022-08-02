QT       -= core gui
CONFIG   -= qt

TARGET = sdl
TEMPLATE = lib
CONFIG += staticlib link_pkgconfig object_parallel_to_source
INCLUDEPATH += include dxsdk/include

# platform-independent
SOURCES += \
    $$files(src/*.c) \
    $$files(src/atomic/*.c) \
    $$files(src/audio/*.c) \
    $$files(src/cpuinfo/*.c) \
    $$files(src/events/*.c) \
    $$files(src/file/*.c) \
    $$files(src/haptic/*.c) \
    $$files(src/joystick/*.c) \
    $$files(src/joystick/steam/*.c) \
    $$files(src/joystick/hidapi/*.c) \
    $$files(src/libm/*.c) \
    $$files(src/loadso/*.c) \
    $$files(src/main/*.c) \
    $$files(src/power/*.c) \
    $$files(src/render/*.c) \
    $$files(src/stdlib/*.c) \
    $$files(src/test/*.c) \
    $$files(src/thread/*.c) \
    $$files(src/timer/*.c) \
    $$files(src/video/*.c) \
    $$files(src/video/yuv2rgb/*.c) \
    $$files(src/dynapi/*.c) \
    $$files(src/sensor/*.c) \
    $$files(src/hidapi/*.c) \
    $$files(src/misc/*.c) \
    $$files(src/locale/*.c)

# core
win32:SOURCES += $$files(src/core/windows/*.c)
linux:SOURCES += \
    src/core/linux/SDL_evdev_capabilities.c \
    src/core/linux/SDL_evdev_kbd.c \
    src/core/linux/SDL_evdev.c \
    src/core/linux/SDL_ime.c \
    src/core/linux/SDL_threadprio.c \
    src/core/linux/SDL_udev.c

# audio
SOURCES += $$files(src/audio/dummy/*.c)

unix:SOURCES += $$files(src/audio/alsa/*.c) \
                $$files(src/audio/bsd/*.c) \
                $$files(src/audio/dsp/*.c) \
                $$files(src/audio/pulseaudio/*.c)
macx:SOURCES  += $$files(src/audio/coreaudio/*.c)
win32:SOURCES += $$files(src/audio/directsound/*.c) \
                 $$files(src/audio/winmm/*.c)

# file
macx:OBJECTIVE_SOURCES += $$files(src/file/cocoa/*.m)

# filesystem
unix:!macx:SOURCES += $$files(src/filesystem/unix/*.c)
macx:OBJECTIVE_SOURCES += $$files(src/filesystem/cocoa/*.m)
win32:SOURCES += $$files(src/filesystem/windows/*.c)

# haptic
SOURCES += $$files(src/haptic/dummy/*.c)

linux:SOURCES += $$files(src/haptic/linux/*.c)
macx:SOURCES  += $$files(src/haptic/darwin/*.c)
win32:SOURCES += $$files(src/haptic/windows/*.c)

# joystick
linux:SOURCES += $$files(src/joystick/linux/*.c)
macx:SOURCES  += $$files(src/joystick/darwin/*.c)
win32:SOURCES += $$files(src/joystick/dummy/*.c)

# loadso
unix|macx:SOURCES += $$files(src/loadso/dlopen/*.c)
win32:SOURCES     += $$files(src/loadso/windows/*.c)

# main
# NOPE

# power
linux:SOURCES += $$files(src/power/linux/*.c)
macx:SOURCES += $$files(src/power/macosx/*.c)
macx:OBJECTIVE_SOURCES += $$files(src/power/macosx/*.m)
win32:SOURCES += $$files(src/power/windows/*.c)

# render
SOURCES += $$files(src/render/software/*.c) \
           $$files(src/render/opengl/*.c)
win32:SOURCES += $$files(src/render/direct3d/*.c)

# thread
win32:SOURCES += $$files(src/thread/windows/*.c) src/thread/generic/SDL_syscond.c
unix|macx:SOURCES += $$files(src/thread/pthread/*.c)

# timer
win32:SOURCES += $$files(src/timer/windows/*.c)
unix:SOURCES += $$files(src/timer/unix/*.c)

# video
SOURCES += $$files(src/video/dummy/*.c)
win32:SOURCES += $$files(src/video/windows/*.c)
unix:!macx:SOURCES += $$files(src/video/x11/*.c)
macx:OBJECTIVE_SOURCES += $$files(src/video/cocoa/*.m)

# sensor
SOURCES += $$files(src/sensor/dummy/*.c)

# hidapi
win32:SOURCES += $$files(src/hidapi/windows/*.c)
linux:SOURCES += $$files(src/hidapi/linux/*.c)
macx:SOURCES += $$files(src/hidapi/mac/*.c)

# misc
win32:SOURCES += $$files(src/misc/windows/*.c)
unix:!macx:SOURCES += $$files(src/misc/unix/*.c)
macx:OBJECTIVE_SOURCES += $$files(src/misc/macosx/*.m)

# locale
win32:SOURCES += $$files(src/locale/windows/*.c)
unix:!macx:SOURCES += $$files(src/locale/unix/*.c)
macx:OBJECTIVE_SOURCES += $$files(src/locale/macosx/*.m)

HEADERS += $$files(include/*.h)

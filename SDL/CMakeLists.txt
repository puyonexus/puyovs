file(GLOB UNIVERSAL_SOURCES
    # Core source
    src/*.c
    src/atomic/*.c
    src/audio/*.c
    src/cpuinfo/*.c
    src/events/*.c
    src/file/*.c
    src/haptic/*.c
    src/joystick/*.c
    src/libm/*.c
    src/loadso/*.c
    src/main/*.c
    src/power/*.c
    src/render/*.c
    src/stdlib/*.c
    src/test/*.c
    src/thread/*.c
    src/timer/*.c
    src/video/*.c

    # Common drivers
    src/audio/dummy/*.c
    src/haptic/dummy/*.c
    src/render/software/*.c
    src/render/opengl/*.c
    src/video/dummy/*.c
)

# Windows drivers
if(WIN32)
    file(GLOB PLATFORM_SOURCES
        src/core/windows/*.c
        src/audio/directsound/*.c
        src/audio/winmm/*.c
        src/haptic/windows/*.c
        src/joystick/*.c
        src/joystick/dummy/*.c
        src/loadso/windows/*.c
        src/power/windows/*.c
        src/render/direct3d/*.c
        src/thread/windows/*.c
        src/thread/generic/SDL_syscond.c
        src/timer/windows/*.c
        src/video/windows/*.c
    )
elseif(APPLE)
    file(GLOB PLATFORM_SOURCES
        src/audio/coreaudio/*.c
        src/file/cocoa/*.m
        src/haptic/darwin/*.c
        src/joystick/darwin/*.c
        src/power/macosx/*.c
        src/power/macosx/*.m
        src/video/cocoa/*.m
        src/loadso/dlopen/*.c
        src/thread/pthread/*.c
        src/timer/unix/*.c
    )
elseif(LINUX)
    file(GLOB PLATFORM_SOURCES
        src/audio/alsa/*.c
        src/audio/bsd/*.c
        src/audio/dsp/*.c
        src/audio/pulseaudio/*.c
        src/joystick/linux/*.c
        src/power/linux/*.c
        src/haptic/linux/*.c
        src/loadso/dlopen/*.c
        src/thread/pthread/*.c
        src/timer/unix/*.c
        src/video/x11/*.c
    )
else()
    file(GLOB PLATFORM_SOURCES
        src/audio/bsd/*.c
        src/audio/dsp/*.c
        src/audio/pulseaudio/*.c
        src/loadso/dlopen/*.c
        src/thread/pthread/*.c
        src/timer/unix/*.c
        src/video/x11/*.c
    )
endif()

include_directories(include dxsdk/include)

add_library(SDL
    ${UNIVERSAL_SOURCES}
    ${PLATFORM_SOURCES}
)

if(WIN32)
    target_link_libraries(SDL Imm32)
elseif(APPLE)
    target_link_libraries(SDL
        "-framework Foundation"
        "-framework Carbon"
        "-framework AGL"
        "-framework OpenGL"
        "-framework ForceFeedback"
        "-framework AudioUnit"
        "-framework CoreAudio"
        "-framework IOKit"
        "-framework AppKit"
        "dl"
        "pthread"
    )
else()
    target_link_libraries(SDL
        "dl"
        "pthread"
    )
endif()

target_include_directories(SDL PUBLIC include)

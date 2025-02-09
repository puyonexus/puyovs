include_directories(. include/alib ../SDL/include)

add_compile_definitions(
    ALIB_OGG_SUPPORT
    ALIB_VGMSTREAM_SUPPORT
    FLOATING_POINT
)

add_library(Audiolib STATIC
    audiolib.cpp
    minimp3.c
    stream.cpp
    resampler.cpp
    mixer.cpp
    buffer.cpp
    samplereader.cpp
    readers/vorbisreader.cpp
    speex/resample.c
    ringbuffer.cpp
    readers/vgmreader.cpp
    readers/pcmreader.cpp
    tinythread.cpp

    minivorbis.cpp
    fast_mutex.h
    minivorbis.h
    minimp3.h
    samplereader.h
    tinythread.h
    readers/vorbisreader.h
    speex/stack_alloc.h
    speex/speex_types.h
    speex/speex_resampler.h
    speex/fixed_generic.h
    speex/fixed_debug.h
    speex/fixed_bfin.h
    speex/fixed_arm5e.h
    speex/fixed_arm4.h
    speex/arch.h
    include/alib/stream.h
    include/alib/sample.h
    include/alib/audiolib.h
    include/alib/resampler.h
    include/alib/mixer.h
    include/alib/buffer.h
    include/alib/ringbuffer.h
    readers/vgmreader.h
    readers/pcmreader.h
    include/alib/common.h
)

target_link_libraries(Audiolib
    $<IF:$<TARGET_EXISTS:SDL2::SDL2>,SDL2::SDL2,SDL2::SDL2-static>
    VgmStream
)
target_include_directories(Audiolib PUBLIC include)
target_compile_features(Audiolib PUBLIC cxx_std_17)

add_library(Audiolib::Audiolib ALIAS Audiolib)


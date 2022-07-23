include_directories(. include/alib ../SDL/include)

add_compile_definitions(
    ALIB_OGG_SUPPORT
    ALIB_VGMSTREAM_SUPPORT
    FLOATING_POINT
)

add_library(Audiolib
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
)

target_link_libraries(Audiolib SDL VgmStream)
target_include_directories(Audiolib PUBLIC include)
target_compile_features(Audiolib PUBLIC cxx_std_17)

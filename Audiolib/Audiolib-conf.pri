CONFIG += link_pkgconfig

# MP3 support
# --
# Note: producing binaries supporting MP3 may be a violation of patent laws
#       under your jurisdiction. Please consult local laws before redistributing
#       binaries produced with MP3 support.
#
# Note: minimp3 source created by Fabrice Bellard, Martin J. Fiedler, et al.
#       and licensed under LGPL 2.1+. When minimp3 is compiled in, you must
#       distribute Audiolib in such a way that the software's local copy of
#       Audiolib can be modified under the terms of the LGPL 2.1+ license.
#DEFINES += ALIB_MP3_SUPPORT

# Vorbis support
# --
# stb_vorbis by Sean Barrett. This code holds no license or patent implications.
DEFINES += ALIB_OGG_SUPPORT

# VgmStream support
DEFINES += ALIB_VGMSTREAM_SUPPORT
include(../VgmStream/VgmStream.pri)

# SPEEX
DEFINES += FLOATING_POINT

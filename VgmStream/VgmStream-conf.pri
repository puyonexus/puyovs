CONFIG += link_pkgconfig

# For Ogg Vorbis support
#packagesExist(vorbis) {
#   PKGCONFIG += vorbis
#   DEFINES += VGM_USE_VORBIS
#}

# For MPEG decoding support
#packagesExist(libmpg123) {
#   PKGCONFIG += libmpg123
#   DEFINES += VGM_USE_MPEG
#}

# For G7221 decoding support
#DEFINES += VGM_USE_G7221
#LIBS += -lg7221_decode

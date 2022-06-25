CONFIG += link_pkgconfig

win32 {
    LIBS += -ldinput8 -ldxguid
    DEFINES += ILIB_USE_DRIVER_DINPUT
} else {
    DEFINES += ILIB_USE_DRIVER_SDL
}

QT -= core gui

TARGET = enet
TEMPLATE = lib
CONFIG += staticlib

INCLUDEPATH += "include"
INCLUDEPATH += "include/enet"

SOURCES += callbacks.c compress.c host.c list.c packet.c peer.c protocol.c
win32 {
    SOURCES += win32.c
} else {
    SOURCES += unix.c
    DEFINES += HAS_SOCKLEN_T
}

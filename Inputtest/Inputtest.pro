TEMPLATE = app
CONFIG += console
CONFIG -= qt

SOURCES += main.cpp

include(../Inputlib/Inputlib.pri)
include(../SDL/SDL.pri)

win32-msvc* {
    CONFIG -= embed_manifest_dll embed_manifest_exe
}

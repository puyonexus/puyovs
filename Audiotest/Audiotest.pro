TEMPLATE = app
CONFIG += console qt
QT += core network

SOURCES += main.cpp

include(../Audiolib/Audiolib.pri)
include(../SDL/SDL.pri)

win32-msvc* {
    CONFIG -= embed_manifest_dll embed_manifest_exe
}

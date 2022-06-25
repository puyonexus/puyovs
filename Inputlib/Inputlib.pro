QT       -= core gui

TARGET = ilib
TEMPLATE = lib
CONFIG += staticlib link_pkgconfig

DEFINES += INPUTLIB

SOURCES += \
    driver.cpp \
    drivers/sdldriver.cpp \
    drivers/dinputdriver.cpp \
    drivers/nulldriver.cpp

HEADERS += \
    include/ilib/inputlib.h \
    include/ilib/driver.h \
    drivers/sdldriver.h \
    include/ilib/inputevent.h \
    drivers/dinputdriver.h \
    js2format.h \
    drivers/nulldriver.h

INCLUDEPATH += . include/ilib dxsdk/include

OTHER_FILES += \
    Inputlib.pri \
    Inputlib-conf.pri

include(../SDL/SDL.pri)
include(Inputlib-conf.pri)

QT       -= core gui

TARGET = puyo
TEMPLATE = lib
CONFIG += staticlib

HEADERS += \
    tinyxml.h \
    tinystr.h \
    sprite.h \
    sound.h \
    ruleset.h \
    puyo.h \
    player.h \
    otherObjects.h \
    nextPuyo.h \
    mt.h \
    movePuyo.h \
    menu.h \
    global.h \
    gameSettings.h \
    game.h \
    fieldProp.h \
    field.h \
    feverCounter.h \
    dropPattern.h \
    dictionary.h \
    controller.h \
    characterSelect.h \
    animation.h \
    AI.h \
    frontend.h \
    classicpool.h \
    classicpuyorng.h \
    classicrng.h \
    legacypuyorng.h \
    puyorng.h

SOURCES += \
    tinyxmlparser.cpp \
    tinyxmlerror.cpp \
    tinyxml.cpp \
    tinystr.cpp \
    sprite.cpp \
    sound.cpp \
    ruleset.cpp \
    puyo.cpp \
    player.cpp \
    otherObjects.cpp \
    nextPuyo.cpp \
    mt.cc \
    movePuyo.cpp \
    menu.cpp \
    main.cpp \
    global.cpp \
    gameSettings.cpp \
    game.cpp \
    field.cpp \
    feverCounter.cpp \
    dropPattern.cpp \
    controller.cpp \
    characterSelect.cpp \
    animation.cpp \
    AI.cpp \
    classicpool.cpp \
    classicpuyorng.cpp \
    classicrng.cpp \
    legacypuyorng.cpp \
    puyorng.cpp

include(../Zlib/Zlib.pri)
include(../ENet/ENet.pri)
include(../PVS_ENet/PVS_ENet.pri)

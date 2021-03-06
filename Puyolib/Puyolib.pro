QT       -= core gui

TARGET = puyo
TEMPLATE = lib
CONFIG += staticlib

HEADERS += \
    tinyxml.h \
    tinystr.h \
    Sprite.h \
    Sound.h \
    RuleSet/RuleSet.h \
    Puyo.h \
    Player.h \
    OtherObjects.h \
    NextPuyo.h \
    MovePuyo.h \
    Menu.h \
    global.h \
    GameSettings.h \
    Game.h \
    FieldProp.h \
    Field.h \
    FeverCounter.h \
    DropPattern.h \
    Controller.h \
    CharacterSelect.h \
    Animation.h \
    AI.h \
    Frontend.h \
    RNG/ClassicPool.h \
    RNG/ClassicPuyoRNG.h \
    RNG/ClassicRNG.h \
    RNG/LegacyPuyoRNG.h \
    RNG/MersenneTwister.h \
    RNG/PuyoRNG.h

SOURCES += \
    tinyxmlparser.cpp \
    tinyxmlerror.cpp \
    tinyxml.cpp \
    tinystr.cpp \
    Sprite.cpp \
    Sound.cpp \
    RuleSet/RuleSet.cpp \
    Puyo.cpp \
    Player.cpp \
    OtherObjects.cpp \
    NextPuyo.cpp \
    MovePuyo.cpp \
    Menu.cpp \
    main.cpp \
    global.cpp \
    GameSettings.cpp \
    Game.cpp \
    Field.cpp \
    FeverCounter.cpp \
    DropPattern.cpp \
    Controller.cpp \
    CharacterSelect.cpp \
    Animation.cpp \
    AI.cpp \
    RNG/ClassicPool.cpp \
    RNG/ClassicPuyoRNG.cpp \
    RNG/ClassicRNG.cpp \
    RNG/LegacyPuyoRNG.cpp \
    RNG/mt.cc \
    RNG/PuyoRNG.cpp

include(../Zlib/Zlib.pri)
include(../ENet/ENet.pri)
include(../PVS_ENet/PVS_ENet.pri)

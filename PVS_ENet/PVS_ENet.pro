QT       -= core gui

TARGET = pvsenet
TEMPLATE = lib
CONFIG += staticlib

SOURCES += \
    PVS_Server.cpp \
    PVS_Peer.cpp \
    PVS_Packet.cpp \
    PVS_Client.cpp \
    PVS_Channel.cpp

HEADERS += \
    PVS_Server.h \
    PVS_Peer.h \
    PVS_Packet.h \
    PVS_Client.h \
    PVS_Channel.h

include(../ENet/ENet.pri)

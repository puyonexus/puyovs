TEMPLATE = subdirs

CONFIG += ordered

SUBDIRS += \
    ENet \
    PVS_ENet \
    Zlib \
    Puyolib \
    SDL \
    VgmStream \
    Audiolib \
    Audiotest \
    Inputlib \
    Inputtest \
    Client

PVS_ENet.depends = ENet
Puyolib.depends = ENet PVS_ENet Zlib
Client.depends = ENet PVS_ENet Puyolib Zlib Audiolib Inputlib PortAudio
Audiotest.depends = Audiolib PortAudio
Inputtest.depends = Inputlib

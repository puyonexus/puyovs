QT += core gui opengl network widgets

# Set target
win32 {
	TARGET = PuyoVS
} else:macx {
    TARGET = PuyoVS
} else {
    TARGET = puyovs
}

CONFIG(release, debug|release):DESTDIR = $${PWD}/../Build/Release
CONFIG(debug, debug|release):DESTDIR = $${PWD}/../Build/Debug

win32 {
    QMAKE_POST_LINK += XCOPY /D/Y/S/I "$$shell_path($${PWD}/../Test)" "$$shell_path($${DESTDIR})"
} else:macx {
    QMAKE_POST_LINK += $${PWD}/../BuildScripts/MacOSX/macx-post-link.sh "$${PWD}/../" "$${DESTDIR}" "$${TARGET}"
} else {
    QMAKE_POST_LINK += cp -R $${PWD}/../Test/* $${DESTDIR}/
}

win32 {
    LIBS += -lopengl32 -lglu32 -ladvapi32
}

TEMPLATE = app

SOURCES += main.cpp \
	mainwindow.cpp \
	chatroomform.cpp \
	settingsdialog.cpp \
    chattextentry.cpp \
    language.cpp \
    netclient.cpp \
    settings.cpp \
    userinfotextbox.cpp \
    createchatroomdialog.cpp \
    gamewidgetgl.cpp \
    offlinedialog.cpp \
    common.cpp \
    gamewidget.cpp \
    gamemanager.cpp \
    inputwidget.cpp \
    inputdialog.cpp \
    chatwindow.cpp \
    mixerwindow.cpp \
    glextensions.cpp \
    fshadergl.cpp \
    glmanager.cpp \
    fimagegl.cpp \
    fsoundalib.cpp \
    ffontgl.cpp \
    frontendgl.cpp \
    gameaudio.cpp \
    replaydialog.cpp \
    admindialog.cpp \
    startupdialog.cpp \
    playlist.cpp \
    musicplayer.cpp \
    pvsapplication.cpp \
    updatedialog.cpp \
    updaterldb.cpp \
    updaterudb.cpp \
    chatroomtextedit.cpp \
    searchdialog.cpp \
    passworddialog.cpp

HEADERS += mainwindow.h \
	chatroomform.h \
	settingsdialog.h \
    chattextentry.h \
    language.h \
    netclient.h \
    settings.h \
    userinfotextbox.h \
    createchatroomdialog.h \
    gamewidgetgl.h \
    offlinedialog.h \
    common.h \
    gamewidget.h \
    gamemanager.h \
    inputwidget.h \
    inputdialog.h \
    keynames.h \
    chatwindow.h \
    mixerwindow.h \
    glextensions.h \
    fshadergl.h \
    glmanager.h \
    fimagegl.h \
    fsoundalib.h \
    ffontgl.h \
    frontendgl.h \
    gameaudio.h \
    replaydialog.h \
    admindialog.h \
    startupdialog.h\
    playlist.h \
    musicplayer.h \
    pvsapplication.h \
    updatedialog.h \
    updaterldb.h \
    updaterudb.h \
    chatroomtextedit.h \
    searchdialog.h \
    passworddialog.h

FORMS += mainwindow.ui \
	chatroomform.ui \
	settingsdialog.ui \
    createchatroomdialog.ui \
    offlinedialog.ui \
    inputwidget.ui \
    inputdialog.ui \
    chatwindow.ui \
    mixerwindow.ui \
    replaydialog.ui \
    admindialog.ui \
    startupdialog.ui \
    updatedialog.ui \
    searchdialog.ui \
    passworddialog.ui

include(../PVS_ENet/PVS_ENet.pri)
include(../ENet/ENet.pri)
include(../JsonCpp/JsonCpp.pri)
include(../Puyolib/Puyolib.pri)
include(../Audiolib/Audiolib.pri)
include(../Inputlib/Inputlib.pri)
include(../SDL/SDL.pri)
include(../Zlib/Zlib.pri)

OTHER_FILES += \
    client.rc \
    puyovs.icns

win32:RC_FILE = client.rc

macx {
    ICON = puyovs.icns
}

RESOURCES += \
    client.qrc

target.path = $${PREFIX}/bin
INSTALLS += target

gamedata.path = $${PREFIX}/share/puyovs
gamedata.files = ../Test/*
INSTALLS += gamedata

defined(${{PREFIX}}) {
    DEFINES += "PREFIX=${{PREFIX}}"
}

equals(${{PACKAGED}}, "true") {
    DEFINES += "PACKAGED"
}

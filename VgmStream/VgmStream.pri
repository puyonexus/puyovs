INCLUDEPATH += $$PWD/include/

win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../VgmStream/release/ -lvgmstream
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../VgmStream/debug/ -lvgmstream
else:unix: LIBS += -L$$OUT_PWD/../VgmStream/ -lvgmstream

DEPENDPATH += $$PWD/

win32-msvc*{
    CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../VgmStream/release/vgmstream.lib
    else:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../VgmStream/debug/vgmstream.lib
}
else:win32 {
    win32:CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../VgmStream/release/libvgmstream.a
    else:win32:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../VgmStream/debug/libvgmstream.a
}
else:unix {
    PRE_TARGETDEPS += $$OUT_PWD/../VgmStream/libvgmstream.a
}

include(VgmStream-conf.pri)

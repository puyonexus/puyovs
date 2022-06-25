win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../Zlib/release/ -lzlib
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../Zlib/debug/ -lzlib
else:unix:!symbian: LIBS += -L$$OUT_PWD/../Zlib/ -lzlib

INCLUDEPATH += $$PWD/
DEPENDPATH += $$PWD/

win32-msvc*{
    CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../Zlib/release/zlib.lib
    else:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../Zlib/debug/zlib.lib
}
else:win32 {
    win32:CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../Zlib/release/libzlib.a
    else:win32:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../Zlib/debug/libzlib.a
}
else:unix {
    PRE_TARGETDEPS += $$OUT_PWD/../Zlib/libzlib.a
}

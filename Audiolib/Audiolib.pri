INCLUDEPATH += $$PWD/include/

win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../Audiolib/release/ -lalib
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../Audiolib/debug/ -lalib
else:unix: LIBS += -L$$OUT_PWD/../Audiolib/ -lalib

DEPENDPATH += $$PWD/

win32-msvc*{
    CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../Audiolib/release/alib.lib
    else:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../Audiolib/debug/alib.lib
}
else:win32 {
    win32:CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../Audiolib/release/libalib.a
    else:win32:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../Audiolib/debug/libalib.a
}
else:unix {
    PRE_TARGETDEPS += $$OUT_PWD/../Audiolib/libalib.a
}

include(Audiolib-conf.pri)

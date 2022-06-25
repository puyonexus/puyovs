include(Inputlib-conf.pri)

INCLUDEPATH += $$PWD/include/

win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../Inputlib/release/ -lilib
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../Inputlib/debug/ -lilib
else:unix: LIBS += -L$$OUT_PWD/../Inputlib/ -lilib

DEPENDPATH += $$PWD/

win32-msvc*{
    CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../Inputlib/release/ilib.lib
    else:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../Inputlib/debug/ilib.lib
}
else:win32 {
    win32:CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../Inputlib/release/libilib.a
    else:win32:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../Inputlib/debug/libilib.a
}
else:unix {
    PRE_TARGETDEPS += $$OUT_PWD/../Inputlib/libilib.a
}

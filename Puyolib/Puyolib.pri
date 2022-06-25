win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../Puyolib/release/ -lpuyo
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../Puyolib/debug/ -lpuyo
else:unix:!symbian: LIBS += -L$$OUT_PWD/../Puyolib/ -lpuyo

INCLUDEPATH += $$PWD/
DEPENDPATH += $$PWD/

win32-msvc*{
    CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../Puyolib/release/puyo.lib
    else:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../Puyolib/debug/puyo.lib
}
else:win32 {
    win32:CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../Puyolib/release/libpuyo.a
    else:win32:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../Puyolib/debug/libpuyo.a
}
else:unix {
    PRE_TARGETDEPS += $$OUT_PWD/../Puyolib/libpuyo.a
}

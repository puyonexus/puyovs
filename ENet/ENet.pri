win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../ENet/release/ -lenet
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../ENet/debug/ -lenet
else:unix: LIBS += -L$$OUT_PWD/../ENet/ -lenet

INCLUDEPATH += $$PWD/include
DEPENDPATH += $$PWD/

win32-msvc*{
    CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../ENet/release/enet.lib
    else:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../ENet/debug/enet.lib
}
else:win32 {
    win32:CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../ENet/release/libenet.a
    else:win32:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../ENet/debug/libenet.a
}
else:unix {
    PRE_TARGETDEPS += $$OUT_PWD/../ENet/libenet.a
}

win32:LIBS += -lws2_32 -lwinmm

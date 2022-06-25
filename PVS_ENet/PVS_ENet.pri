win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../PVS_ENet/release/ -lpvsenet
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../PVS_ENet/debug/ -lpvsenet
else:unix:!symbian: LIBS += -L$$OUT_PWD/../PVS_ENet/ -lpvsenet

INCLUDEPATH += $$PWD/
DEPENDPATH += $$PWD/

win32-msvc*{
    CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../PVS_ENet/release/pvsenet.lib
    else:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../PVS_ENet/debug/pvsenet.lib
}
else:win32 {
    win32:CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../PVS_ENet/release/libpvsenet.a
    else:win32:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../PVS_ENet/debug/libpvsenet.a
}
else:unix {
    PRE_TARGETDEPS += $$OUT_PWD/../PVS_ENet/libpvsenet.a
}

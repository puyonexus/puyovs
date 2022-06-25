INCLUDEPATH += $$PWD/include/

win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../SDL/release/ -lsdl -lgdi32 -luser32 -lwinmm -lole32 -limm32 -lversion -loleaut32 -luuid -lshell32
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../SDL/debug/ -lsdl -lgdi32 -luser32 -lwinmm -lole32 -limm32 -lversion -loleaut32 -luuid -lshell32
else:unix: LIBS += -L$$OUT_PWD/../SDL/ -lsdl -ldl -lpthread

macx: LIBS += -framework Foundation -framework Carbon -framework AGL -framework OpenGL -framework ForceFeedback -framework AudioUnit -framework CoreAudio -framework IOKit -framework AppKit -L$$OUT_PWD/../SDL/ -lsdl

DEPENDPATH += $$PWD/

win32-msvc*{
    CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../SDL/release/sdl.lib
    else:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../SDL/debug/sdl.lib
}
else:win32 {
    CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../SDL/release/libsdl.a
    else:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../SDL/debug/libsdl.a
}
else:unix {
    PRE_TARGETDEPS += $$OUT_PWD/../SDL/libsdl.a
}

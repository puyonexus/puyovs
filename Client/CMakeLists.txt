find_package(Qt5 COMPONENTS Widgets OpenGL Network REQUIRED)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)

add_executable(PuyoVS WIN32
    main.cpp
	mainwindow.cpp
	chatroomform.cpp
	settingsdialog.cpp
    chattextentry.cpp
    language.cpp
    netclient.cpp
    settings.cpp
    userinfotextbox.cpp
    createchatroomdialog.cpp
    gamewidgetgl.cpp
    offlinedialog.cpp
    common.cpp
    gamewidget.cpp
    gamemanager.cpp
    inputwidget.cpp
    inputdialog.cpp
    chatwindow.cpp
    mixerwindow.cpp
    glextensions.cpp
    fshadergl.cpp
    glmanager.cpp
    fimagegl.cpp
    fsoundalib.cpp
    ffontgl.cpp
    frontendgl.cpp
    gameaudio.cpp
    replaydialog.cpp
    admindialog.cpp
    startupdialog.cpp
    playlist.cpp
    musicplayer.cpp
    pvsapplication.cpp
    updatedialog.cpp
    updaterldb.cpp
    updaterudb.cpp
    chatroomtextedit.cpp
    searchdialog.cpp
    passworddialog.cpp
)

if(WIN32)
    target_sources(PuyoVS PRIVATE client.rc)
endif()

set_target_properties(PuyoVS PROPERTIES
    MACOSX_BUNDLE TRUE
)

target_link_libraries(PuyoVS
    PVS_ENet
    JsonCpp
    Puyolib
    Audiolib
    Inputlib
    SDL
    Zlib
    Qt5::Widgets
    Qt5::OpenGL
    Qt5::Network
)

# Try to work around UIC issue on Linux.
target_include_directories(PuyoVS PRIVATE .)

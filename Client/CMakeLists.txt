find_package(Qt5 COMPONENTS Widgets OpenGL Network REQUIRED)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

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
    telemetrydialog.cpp
    playlist.cpp
    musicplayer.cpp
    pvsapplication.cpp
    updatedialog.cpp
    updaterldb.cpp
    updaterudb.cpp
    chatroomtextedit.cpp
    searchdialog.cpp
    passworddialog.cpp

    mainwindow.h
    chatroomform.h
    settingsdialog.h
    chattextentry.h
    language.h
    netclient.h
    settings.h
    userinfotextbox.h
    createchatroomdialog.h
    gamewidgetgl.h
    offlinedialog.h
    common.h
    gamewidget.h
    gamemanager.h
    inputwidget.h
    inputdialog.h
    keynames.h
    chatwindow.h
    mixerwindow.h
    glextensions.h
    fshadergl.h
    glmanager.h
    fimagegl.h
    fsoundalib.h
    ffontgl.h
    frontendgl.h
    gameaudio.h
    replaydialog.h
    admindialog.h
    startupdialog.h
    playlist.h
    musicplayer.h
    pvsapplication.h
    updatedialog.h
    updaterldb.h
    updaterudb.h
    chatroomtextedit.h
    searchdialog.h
    passworddialog.h
    telemetrydialog.h

    mainwindow.ui
    chatroomform.ui
    settingsdialog.ui
    createchatroomdialog.ui
    offlinedialog.ui
    inputwidget.ui
    inputdialog.ui
    chatwindow.ui
    mixerwindow.ui
    replaydialog.ui
    admindialog.ui
    startupdialog.ui
    updatedialog.ui
    searchdialog.ui
    passworddialog.ui
    telemetrydialog.ui

    client.qrc
)

source_group("UI Files" REGULAR_EXPRESSION "\\.ui$")

set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT PuyoVS)

if(WIN32)
    target_sources(PuyoVS PRIVATE client.rc)
elseif(APPLE)
    target_sources(PuyoVS PRIVATE PuyoVS.icns)
    set_source_files_properties(PuyoVS.icns PROPERTIES MACOSX_PACKAGE_LOCATION Resources)
else()
    target_link_libraries(PuyoVS GL)
endif()

set_target_properties(PuyoVS PROPERTIES
    MACOSX_BUNDLE TRUE
    MACOSX_BUNDLE_INFO_PLIST ${CMAKE_CURRENT_SOURCE_DIR}/Info.plist.in
)

target_link_libraries(PuyoVS
    PVS_ENet
	JsonCpp::JsonCpp
    Puyolib::Puyolib
	Audiolib::Audiolib
	Inputlib::Inputlib
    $<IF:$<TARGET_EXISTS:SDL2::SDL2>,SDL2::SDL2,SDL2::SDL2-static>
    ZLIB::ZLIB
    Qt5::Widgets
    Qt5::OpenGL
    Qt5::Network
)

# Try to work around UIC issue on Linux.
target_include_directories(PuyoVS PRIVATE .)

target_compile_features(PuyoVS PUBLIC cxx_std_17)

install(TARGETS PuyoVS DESTINATION ${bindir})
install(DIRECTORY ../Assets/ DESTINATION ${datadir})

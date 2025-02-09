add_library(Puyolib STATIC
    tinyxmlparser.cpp
    tinyxmlerror.cpp
    tinyxml.cpp
    tinystr.cpp
    Sprite.cpp
    Sound.cpp
    RuleSet/RuleSet.cpp
    Puyo.cpp
    Player.cpp
    OtherObjects.cpp
    NextPuyo.cpp
    MovePuyo.cpp
    Menu.cpp
    main.cpp
    global.cpp
    GameSettings.cpp
    Game.cpp
    Field.cpp
    FeverCounter.cpp
    DropPattern.cpp
    Controller.cpp
    CharacterSelect.cpp
    Animation.cpp
    AI.cpp
    RNG/ClassicPool.cpp
    RNG/ClassicPuyoRng.cpp
    RNG/ClassicRng.cpp
    RNG/LegacyPuyoRng.cpp
    RNG/MersenneTwister.cpp
    RNG/PuyoRng.cpp
    DebugLog.cpp
    DebugLog.h
    AssetManager.cpp
    AssetManager.h
    AssetBundle.cpp
    AssetBundle.h)

target_compile_features(Puyolib PUBLIC cxx_std_17)

target_link_libraries(Puyolib ZLIB::ZLIB)

add_library(Puyolib::Puyolib ALIAS Puyolib)

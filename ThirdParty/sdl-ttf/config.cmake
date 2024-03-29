find_package(SDL2_ttf CONFIG)
if(NOT SDL2_ttf_FOUND)
  option(SDL2TTF_VENDORED "" ON)
  option(SDL2TTF_INSTALL "" OFF)
  option(SDL2TTF_SAMPLES "" OFF)
  set(BUILD_SHARED_LIBS OFF)
  set(SKIP_INSTALL_ALL ON)
  option(FT_REQUIRE_ZLIB "" ON)
  option(FT_DISABLE_ZLIB "" OFF)
  add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/sdl-ttf/sdl-ttf)
endif()

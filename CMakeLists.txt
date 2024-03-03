cmake_minimum_required(VERSION 3.24)
project(puyovs)

set(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/CMake/" ${CMAKE_MODULE_PATH})

# Improves IDE support.
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# Path setup.
if(WIN32)
  if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    set(CMAKE_INSTALL_PREFIX "C:\\PuyoVS" CACHE PATH "default install path" FORCE)
  endif()
  set(bindir  ${CMAKE_INSTALL_PREFIX} CACHE PATH "bindir")
  set(datadir ${CMAKE_INSTALL_PREFIX} CACHE PATH "datadir")
else()
  set(bindir  ${CMAKE_INSTALL_PREFIX}/bin          CACHE PATH "bindir")
  set(datadir ${CMAKE_INSTALL_PREFIX}/share/puyovs CACHE PATH "datadir")
endif()

add_definitions("-DPREFIX=\"${CMAKE_INSTALL_PREFIX}\"")

# Options
option(ENABLE_UPDATER "Enables the Puyo VS auto-updater" OFF)
option(ENABLE_ASAN "Enables AddressSanitizer" OFF)

if(ENABLE_UPDATER)
  message(STATUS "Enabling Puyo VS auto-updater")
  add_definitions(-DPUYOVS_UPDATER_ENABLED=1)
endif()

if(ENABLE_ASAN)
  message(STATUS "Enabling ASAN")

  if(MSVC)
    # Disable <vector> ASAN. Otherwise Qt needs ASAN too.
    add_definitions(-D_DISABLE_VECTOR_ANNOTATION)

    set(ASAN_LIB_ARCH ${MSVC_CXX_ARCHITECTURE_ID})
    string(TOLOWER ${ASAN_LIB_ARCH} ASAN_LIB_ARCH)
    if(ASAN_LIB_ARCH STREQUAL "x86")
      set(ASAN_LIB_ARCH "i386")
    elseif(ASAN_LIB_ARCH STREQUAL "x64")
      set(ASAN_LIB_ARCH "x86_64")
    endif()

    link_libraries(clang_rt.asan_dynamic-${ASAN_LIB_ARCH} clang_rt.asan_dynamic_runtime_thunk-${ASAN_LIB_ARCH})
    add_link_options(/incremental:no /wholearchive:clang_rt.asan_dynamic_runtime_thunk-${ASAN_LIB_ARCH}.lib)
  else()
    add_compile_options(-fsanitize=address -fno-omit-frame-pointer)
    add_link_options(-fsanitize=address)
  endif()
endif()

if(MSVC)
  # Disable inactionable warnings.
  # Secure CRT functions are not portable, so we won't use them.
  add_definitions(-D_CRT_SECURE_NO_WARNINGS)
endif()

set(OPT_DEF_LIBC ON)

# Dependencies
include(FetchContent)

# ZLib
find_package(ZLIB)
if(NOT ZLIB_FOUND)
  FetchContent_Declare(
    zlib-ng
    GIT_REPOSITORY https://github.com/zlib-ng/zlib-ng.git
    GIT_TAG        b56a2fd0b126cfe5f13e68ab9090cd4f6a773286 # 2.0.6
  )
  set(ZLIB_ENABLE_TESTS OFF)
  set(ZLIB_COMPAT ON)
  set(SKIP_INSTALL_ALL ON)
  FetchContent_MakeAvailable(zlib-ng)
  add_library(ZLIB::ZLIB ALIAS zlib)
  set(ZLIB_INCLUDE_DIR ${zlib-ng_SOURCE_DIR} CACHE STRING "Path to zlib include directory" FORCE)
  set(ZLIB_LIBRARY ZLIB::ZLIB CACHE STRING "Path to zlib library" FORCE)
  set(ZLIB_FOUND)
  set(CMAKE_DISABLE_FIND_PACKAGE_ZLIB ON)
endif()

# glm
find_package(glm CONFIG)
if(NOT glm_FOUND)
  FetchContent_Declare(
    glm
    GIT_REPOSITORY https://github.com/g-truc/glm.git
    GIT_TAG        cc98465e3508535ba8c7f6208df934c156a018dc
  )
  FetchContent_MakeAvailable(glm)
endif()

# JsonCpp
find_package(jsoncpp CONFIG)
if(NOT jsoncpp_FOUND)
  FetchContent_Declare(
    jsoncpp
    GIT_REPOSITORY https://github.com/open-source-parsers/jsoncpp.git
    GIT_TAG        5defb4ed1a4293b8e2bf641e16b156fb9de498cc # 1.9.5
  )
  FetchContent_MakeAvailable(jsoncpp)
  add_library(JsonCpp::JsonCpp ALIAS jsoncpp_static)
endif()

# SDL2
find_package(SDL2 CONFIG)
if(NOT SDL2_FOUND)
  FetchContent_Declare(
    SDL2
    GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
    GIT_TAG        8c9beb0c873f6ca5efbd88f1ad2648bfc793b2ac # release-2.24.0
  )
  option(SDL2_DISABLE_SDL2MAIN "" OFF)
  option(SDL2_DISABLE_INSTALL "" ON)
  option(SDL2_DISABLE_UNINSTALL "" ON)
  set(SDL_SHARED OFF)
  set(SDL_SHARED_ENABLED_BY_DEFAULT OFF)
  set(SDL_STATIC ON)
  set(SDL_STATIC_ENABLED_BY_DEFAULT ON)
  set(SDL_TEST OFF)
  set(SDL_TEST_ENABLED_BY_DEFAULT OFF)
  FetchContent_MakeAvailable(SDL2)
endif()

# SDL2-ttf
find_package(SDL2_ttf CONFIG)
if(NOT SDL2_ttf_FOUND)
  FetchContent_Declare(
    SDL2_ttf
    GIT_REPOSITORY https://github.com/libsdl-org/SDL_ttf.git
	GIT_TAG        4a318f8dfaa1bb6f10e0c5e54052e25d3c7f3440 # release-2.22.0
  )
  option(SDL2TTF_VENDORED "" ON)
  option(SDL2TTF_INSTALL "" OFF)
  option(SDL2TTF_SAMPLES "" OFF)
  set(BUILD_SHARED_LIBS OFF)
  set(SKIP_INSTALL_ALL ON)
  option(FT_REQUIRE_ZLIB "" OFF)
  option(FT_DISABLE_ZLIB "" OFF)
  FetchContent_MakeAvailable(SDL2_ttf)
endif()

# libspng
find_package(spng)
if(NOT spng_FOUND)
  FetchContent_Declare(
    spng
    GIT_REPOSITORY https://github.com/randy408/libspng.git
    GIT_TAG        adc94393dbeddf9e027d1b2dfff7c1bab975224e
  )
  FetchContent_MakeAvailable(spng)
  add_library(spng::spng ALIAS spng)
endif()

# TODO: VgmStream does not install development files, so it is always vendored.
add_subdirectory(ThirdParty/vgmstream)

# TODO: ENet is ~unmaintained, so it is always vendored.
add_subdirectory(ThirdParty/enet)

# Add sub-projects.
add_subdirectory(Audiolib)
add_subdirectory(Audiotest)
add_subdirectory(Client)
add_subdirectory(ClientNG)
add_subdirectory(Inputlib)
add_subdirectory(Inputtest)
add_subdirectory(Puyolib)
add_subdirectory(PVS_ENet)

# CPack support.
set(CPACK_SET_DESTDIR                 ON)
set(CPACK_PACKAGE_NAME                "puyovs")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "A Puyo Puyo fan game")
set(CPACK_PACKAGE_VENDOR              "Puyo Nexus")
set(CPACK_PACKAGE_VERSION_MAJOR       "32")
set(CPACK_PACKAGE_VERSION_MINOR       "0")
set(CPACK_PACKAGE_VERSION_PATCH       "0")
set(CPACK_RPM_PACKAGE_GROUP           "Amusements/Games")
set(CPACK_RPM_PACKAGE_LICENSE         "GPLv3")
set(CPACK_DEBIAN_PACKAGE_SECTION      "Games")
set(CPACK_SOURCE_GENERATOR            "TGZ;TBZ2;ZIP")
set(CPACK_SOURCE_IGNORE_FILES         "\\\\.#;/#;.*~;\\\\.swp;/\\\\.git")
list(APPEND CPACK_SOURCE_IGNORE_FILES "${CMAKE_BINARY_DIR}")
include(CPack)

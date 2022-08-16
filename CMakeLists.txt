cmake_minimum_required(VERSION 3.13)
project(puyovs)

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

option(SDL2_DISABLE_SDL2MAIN "" OFF)
option(SDL2_DISABLE_INSTALL "" ON)
option(SDL2_DISABLE_UNINSTALL "" ON)
set(SDL_SHARED OFF)
set(SDL_SHARED_ENABLED_BY_DEFAULT OFF)
set(SDL_STATIC ON)
set(SDL_STATIC_ENABLED_BY_DEFAULT ON)
set(SDL_TEST OFF)
set(SDL_TEST_ENABLED_BY_DEFAULT OFF)
set(OPT_DEF_LIBC ON)

# Add sub-projects.
add_subdirectory(Audiolib)
add_subdirectory(Audiotest)
add_subdirectory(Client)
add_subdirectory(ENet)
add_subdirectory(Inputlib)
add_subdirectory(Inputtest)
add_subdirectory(JsonCpp)
add_subdirectory(Puyolib)
add_subdirectory(PVS_ENet)
add_subdirectory(SDL)
add_subdirectory(VgmStream)

add_subdirectory(ThirdParty/glm)
add_subdirectory(ThirdParty/zlib-ng)
add_subdirectory(ThirdParty/libspng)
add_subdirectory(ThirdParty/sdl-ttf)

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

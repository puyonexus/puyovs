cmake_minimum_required(VERSION 3.13)
project(puyovs)

set(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/CMakeModules/" ${CMAKE_MODULE_PATH})

# Improves IDE support.
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# Automatically set up compile_commands.json
option(SETUP_COMPILE_COMMANDS "Automatically set up compile_commands.json" ON)
if(SETUP_COMPILE_COMMANDS)
  set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
  execute_process(
    COMMAND ${CMAKE_COMMAND} -E create_symlink
      ${CMAKE_BINARY_DIR}/compile_commands.json
      ${CMAKE_CURRENT_SOURCE_DIR}/compile_commands.json
  )
endif()

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

include(ThirdParty/zlib-ng/config.cmake)
include(ThirdParty/glm/config.cmake)
include(ThirdParty/jsoncpp/config.cmake)
include(ThirdParty/libspng/config.cmake)
include(ThirdParty/sdl/config.cmake)
include(ThirdParty/sdl-ttf/config.cmake)
include(ThirdParty/vgmstream/config.cmake)
include(ThirdParty/enet/config.cmake)

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

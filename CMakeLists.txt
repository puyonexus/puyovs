cmake_minimum_required(VERSION 3.10)
project(puyovs)

set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MD")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MD")

# Improves IDE support.
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# Path setup.
set(bindir  ${CMAKE_INSTALL_PREFIX}/bin          CACHE PATH "bindir")
set(datadir ${CMAKE_INSTALL_PREFIX}/share/puyovs CACHE PATH "datadir")

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
add_subdirectory(Zlib)

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

find_package(PkgConfig)
pkg_check_modules(PC_spng QUIET spng)

find_path(spng_INCLUDE_DIRS
  NAMES spng.h
  PATHS ${PC_spng_INCLUDE_DIRS}
)
find_library(spng_LIBRARY
  NAMES spng libspng
  PATHS ${PC_spng_LIBRARY_DIRS}
)
set(spng_VERSION ${PC_spng_VERSION})
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(spng
  FOUND_VAR spng_FOUND
  REQUIRED_VARS
    spng_LIBRARY
    spng_INCLUDE_DIRS
  VERSION_VAR spng_VERSION
)
if(spng_FOUND)
  add_library(spng::spng IMPORTED SHARED)
  set_target_properties(spng::spng PROPERTIES
    IMPORTED_LOCATION ${spng_LIBRARY}
    INTERFACE_INCLUDE_DIRECTORIES ${spng_INCLUDE_DIRS}
  )
endif()

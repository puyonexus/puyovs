find_package(spng)
if(NOT spng_FOUND)
  add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/libspng)
endif()

find_package(glm CONFIG)
if(NOT glm_FOUND)
  add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/glm/glm)
endif()

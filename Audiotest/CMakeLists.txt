find_package(Qt5 COMPONENTS Network REQUIRED)
add_executable(Audiotest main.cpp)
target_link_libraries(Audiotest Audiolib Qt5::Network)

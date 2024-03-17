find_package(Qt5 COMPONENTS Network REQUIRED)
add_executable(Audiotest main.cpp)
target_link_libraries(Audiotest Audiolib::Audiolib Qt5::Network)
target_compile_features(Audiotest PUBLIC cxx_std_17)

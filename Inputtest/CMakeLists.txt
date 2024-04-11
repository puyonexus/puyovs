include_directories(../Inputlib/include)
add_executable(Inputtest main.cpp)
target_link_libraries(Inputtest Inputlib::Inputlib)
target_compile_features(Inputtest PUBLIC cxx_std_17)

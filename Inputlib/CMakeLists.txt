add_compile_definitions(INPUTLIB)

add_library(Inputlib
    driver.cpp
    drivers/sdldriver.cpp
    drivers/dinputdriver.cpp
    drivers/nulldriver.cpp
)

if(WIN32)
    add_compile_definitions(ILIB_USE_DRIVER_DINPUT)
    target_link_libraries(Inputlib PUBLIC dinput8 dxguid)
    include_directories(dxsdk/include)
else()
    add_compile_definitions(ILIB_USE_DRIVER_SDL)
endif()

include_directories(. include/ilib)
target_include_directories(Inputlib PUBLIC include)
target_link_libraries(Inputlib PUBLIC SDL)

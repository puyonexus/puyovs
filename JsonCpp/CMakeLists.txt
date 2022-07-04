add_library(JsonCpp
    jsoncpp.cpp
)
target_compile_definitions(JsonCpp PUBLIC JSON_IS_AMALGAMATION)
target_include_directories(JsonCpp PUBLIC .)

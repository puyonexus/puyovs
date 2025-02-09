include_directories(include)

add_library(ENet STATIC
    callbacks.c
    compress.c
    host.c
    list.c
    packet.c
    peer.c
    protocol.c
)

if(WIN32)
    target_sources(ENet PRIVATE win32.c)
	target_link_libraries(ENet ws2_32.lib)
else()
    target_sources(ENet PRIVATE unix.c)
    add_compile_definitions(HAS_SOCKLEN_T)
endif()

target_include_directories(ENet PUBLIC include)

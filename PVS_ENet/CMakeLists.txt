add_library(PVS_ENet STATIC
    PVS_Channel.cpp
    PVS_Channel.h
    PVS_Client.cpp
    PVS_Client.h
    PVS_Packet.cpp
    PVS_Packet.h
    PVS_Peer.cpp
    PVS_Peer.h
    PVS_Server.cpp
    PVS_Server.h
)

target_link_libraries(PVS_ENet ENet)
target_include_directories(PVS_ENet PUBLIC .)

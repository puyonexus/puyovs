#pragma once

#include <iostream>
#include <cstdlib>
#include <stdio.h>
#include <string>
#include <list>

struct _ENetPeer;

//the struct PVS_Peer contains the PuyoVS specific values for a peer
//add it to ENetPeer::data
struct PVS_Peer
{
    PVS_Peer();
    struct _ENetPeer* enetpeer;
    unsigned int id;
    std::string name;
    std::string oldName;
    std::list<std::string> channels; //this list is maintained by the channel functions
};

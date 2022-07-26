#pragma once

#include <cstdlib>
#include <iostream>
#include <list>
#include <stdio.h>
#include <string>

struct _ENetPeer;

// The struct PVS_Peer contains the PuyoVS specific values for a peer
// Add it to ENetPeer::data
struct PVS_Peer {
	PVS_Peer();
	_ENetPeer* enetpeer;
	unsigned int id;
	std::string name;
	std::string oldName;
	std::list<std::string> channels; // This list is maintained by the channel functions
};

#pragma once

#include <iostream>
#include <cstdlib>
#include <stdio.h>
#include <string>
#include <list>

struct _ENetPeer;

// The struct PVS_Peer contains the PuyoVS specific values for a peer
// Add it to ENetPeer::data
struct PVS_Peer
{
	PVS_Peer();
	_ENetPeer* enetpeer;
	unsigned int id;
	std::string name;
	std::string oldName;
	std::list<std::string> channels; // This list is maintained by the channel functions
};

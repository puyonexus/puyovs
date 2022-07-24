#include "PVS_Server.h"

#define ERROR_PACKET 1
#define ERROR_OTHER 2

PVS_Server::PVS_Server()
{
	host = nullptr;
	id_counter = 1;
}

void PVS_Server::checkEvent()
{
	ENetEvent event;

	// Processing incoming events:
	while (enet_host_service(host, &event, 1) > 0)
	{
		switch (event.type)
		{
		case ENET_EVENT_TYPE_CONNECT:
			connect(event);
			break;
		case ENET_EVENT_TYPE_RECEIVE:
			if (receive(event) == 0)
				onReceive();
			else if (receive(event) == ERROR_PACKET)
			{
				sprintf(currentErrorString, "Error at reading or writing packet.");
				onError();
			}
			else if (receive(event) == ERROR_OTHER)
			{
				sprintf(currentErrorString, "Bad or strange request from peer.");
				onError();
			}
			enet_packet_destroy(event.packet);
			break;
		case ENET_EVENT_TYPE_DISCONNECT:
			disconnect(event);
			break;
		default:
			break;
		}
	}
}

bool PVS_Server::initNetwork()
{
	if (enet_initialize() != 0)
		return false;
	atexit(enet_deinitialize);

	// Bind the server to the default localhost.
	// A specific host address can be specified by
	// enet_address_set_host (& address, "x.x.x.x");

	address.port = 2424;
	address.host = ENET_HOST_ANY;
	host = enet_host_create(
		&address, // The address to bind the server host to
		N_MAXCLIENTS, // Number of clients
		N_CHANNELS, // Number of channels
		0, // Assume any amount of incoming bandwidth
		0 // Assume any amount of outgoing bandwidth
	);
	if (!host)
		return false;

	onInit();
	return true;
}

void PVS_Server::sendPacket(int channelNum, char* pack, int len, ENetPeer* peer) const
{
	/* Create a reliable packet of size 7 containing "packet\0" */
	ENetPacket* packet = enet_packet_create(pack, len, ENET_PACKET_FLAG_RELIABLE);

	/* Send the packet to the peer over channel id 0. */
	/* One could also broadcast the packet by         */
	/* enet_host_broadcast (host, 0, packet);         */
	enet_peer_send(peer, channelNum, packet);
	enet_host_flush(host);
}

// A peer connected
void PVS_Server::connect(ENetEvent& event)
{
	// Add to list
	enetPeerList.push_back(event.peer);
	// Initialize data
	event.peer->data = new PVS_Peer;
	currentPVS_Peer = static_cast<PVS_Peer*>(event.peer->data);
	currentPVS_Peer->id = id_counter;
	currentPVS_Peer->enetpeer = event.peer;

	// Send connected confirmation
	packetWriter pw(sizeof(unsigned char) + sizeof(unsigned int));
	pw.writeValue(static_cast<unsigned char>(PT_CONNECT));
	pw.writeValue(id_counter);
	sendPacket(0, pw.getArray(), pw.getLength(), event.peer);
	id_counter++;
	onConnect(event);
	// Send a channellist
	sendChannelList(event.peer);
}

// A peer disconnected
void PVS_Server::disconnect(ENetEvent& event)
{
	currentPVS_Peer = getPVS_Peer(event.peer);

	// Callback before doing anything
	onDisconnect(event);

	// Remove from channels
	std::list<std::string>& cl = currentPVS_Peer->channels;
	while (!cl.empty())
	{
		currentChannelName = *cl.begin();
		channelManager.removePeer(*cl.begin(), currentPVS_Peer);
		removePeerFromChannelMessage(currentChannelName, currentPVS_Peer);
	}

	// Delete PVS_Peer
	delete currentPVS_Peer;
	event.peer->data = nullptr;

	// Remove from global peerlist
	enetPeerList.erase(std::find(enetPeerList.begin(), enetPeerList.end(), event.peer));
}

int PVS_Server::receive(ENetEvent& event)
{
	// Process package
	// Get first byte
	packetReader pr(event.packet);
	unsigned char type = 0;
	if (!pr.getValue(type)) return ERROR_PACKET;
	switch (type)
	{
	case PT_CONNECT:
	{
		// Use for debugging (the client should never send on the 0 channel)
		// Check packet
		std::string mes;
		if (!pr.getString(mes)) return ERROR_PACKET;

		puts(mes.c_str());
		fflush(stdout);
		break;
	}
	case PT_MESSERVER:
	{
		// Send string to/from server
		// Check packet
		unsigned int id = 0;
		if (!pr.getValue(id)) return ERROR_PACKET;
		if (!pr.getValue(subChannel)) return ERROR_PACKET;
		if (!pr.getString(currentString)) return ERROR_PACKET;

		// Select peer
		currentPVS_Peer = getPVS_PeerFromID(id);
		onMessageServer();
		break;
	}
	case PT_MESCHANNEL:
	{
		// Send string to/from channel
		// Check packet
		unsigned int id = 0;
		if (!pr.getValue(id)) return ERROR_PACKET;
		if (!pr.getValue(subChannel)) return ERROR_PACKET;
		std::string channel;
		if (!pr.getString(channel)) return ERROR_PACKET;
		if (!pr.getString(currentString)) return ERROR_PACKET;

		// Does channel exist?
		if (!channelManager.channelExists(channel))
			return ERROR_OTHER;
		// Check if peer is in channel
		if (!channelManager.peerExistsInChannel(channel, getPVS_PeerFromID(id)))
			return ERROR_OTHER;

		// Write a new packet
		packetWriter pw(sizeof(unsigned char) + sizeof(unsigned int) + 1 + channel.length() + 1 + currentString.length() + 1);
		pw.writeValue(static_cast<unsigned char>(PT_MESCHANNEL));
		pw.writeValue(id);
		pw.writeValue(subChannel);
		pw.writeString(channel.c_str());
		pw.writeString(currentString.c_str());

		// Pass it to all peers in channel, except the sender
		peerList* pl = channelManager.getChannel(channel)->peers;
		peerList::iterator it;
		for (it = pl->begin(); it != pl->end(); it++)
		{
			if ((*it)->id == id)
				continue;
			sendPacket(0, pw.getArray(), pw.getLength(), (*it)->enetpeer);
		}
		break;
	}
	case PT_MESCHANNELPEER:
	{
		// Send string to/from peer
		unsigned int id = 0;
		if (!pr.getValue(id)) return ERROR_PACKET;
		if (!pr.getValue(subChannel)) return ERROR_PACKET;
		unsigned int targetID = 0;
		if (!pr.getValue(targetID)) return ERROR_PACKET;
		if (!pr.getString(currentChannelName)) return ERROR_PACKET;
		if (!pr.getString(currentString)) return ERROR_PACKET;

		// Does channel exist?
		if (!channelManager.channelExists(currentChannelName))
			return ERROR_OTHER;
		// Check if peers are in channel
		if (!channelManager.peerExistsInChannel(currentChannelName, getPVS_PeerFromID(id)))
			return ERROR_OTHER;
		if (!channelManager.peerExistsInChannel(currentChannelName, getPVS_PeerFromID(targetID)))
			return  ERROR_OTHER;

		// Target and peer are the same
		if (id == targetID) return ERROR_OTHER;

		// Write a new packet
		packetWriter pw(sizeof(unsigned char) + sizeof(unsigned int) + 1 + currentChannelName.length() + 1 + currentString.length() + 1);
		pw.writeValue(static_cast<unsigned char>(PT_MESCHANNELPEER));
		pw.writeValue(id);
		pw.writeValue(subChannel);
		pw.writeString(currentChannelName.c_str());
		pw.writeString(currentString.c_str());

		sendPacket(0, pw.getArray(), pw.getLength(), getPeerFromID(targetID));
		break;
	}
	case PT_RAWMESCHANNEL:
	{
		// Send raw packet to/from channel
		// Check packet
		unsigned int id = 0;
		if (!pr.getValue(id)) return ERROR_PACKET;
		if (!pr.getValue(subChannel)) return ERROR_PACKET;
		std::string channel;
		if (!pr.getString(channel)) return ERROR_PACKET;
		unsigned int length = 0;
		if (!pr.getValue(length)) return ERROR_PACKET;
		if (length == 0) return ERROR_PACKET;

		// Does channel exist?
		if (!channelManager.channelExists(channel))
			return ERROR_OTHER;
		// Check if peer is in channel
		if (!channelManager.peerExistsInChannel(channel, getPVS_PeerFromID(id)))
			return ERROR_OTHER;

		// Write a new packet
		packetWriter pw(sizeof(unsigned char) + sizeof(unsigned int) + 1 + channel.length() + 1 + sizeof(unsigned int) + length);
		pw.writeValue(static_cast<unsigned char>(PT_MESCHANNEL));
		pw.writeValue(id);
		pw.writeValue(subChannel);
		pw.writeString(channel.c_str());
		pw.writeValue(length);
		// Copy char array
		if (!pw.copyChars(pr.getChars(length), length)) return ERROR_PACKET;

		// Pass it to all peers in channel, except the sender
		peerList* pl = channelManager.getChannel(channel)->peers;
		peerList::iterator it;
		for (it = pl->begin(); it != pl->end(); it++)
		{
			if ((*it)->id == id)
				continue;
			sendPacket(0, pw.getArray(), pw.getLength(), (*it)->enetpeer);
		}
		break;
	}
	case PT_RAWMESCHANNELPEER:
	{
		// Send char array to/from peer
		unsigned int id = 0;
		if (!pr.getValue(id)) return ERROR_PACKET;
		if (!pr.getValue(subChannel)) return ERROR_PACKET;
		unsigned int targetID = 0;
		if (!pr.getValue(targetID)) return ERROR_PACKET;
		if (!pr.getString(currentChannelName)) return ERROR_PACKET;
		unsigned int length = 0;
		if (!pr.getValue(length)) return ERROR_PACKET;
		if (length == 0) return ERROR_PACKET;

		// Does channel exist?
		if (!channelManager.channelExists(currentChannelName))
			return ERROR_OTHER;
		// Check if peer is in channel
		if (!channelManager.peerExistsInChannel(currentChannelName, getPVS_PeerFromID(id)))
			return ERROR_OTHER;

		// Write a new packet
		packetWriter pw(sizeof(unsigned char) + sizeof(unsigned int) + 1 + currentChannelName.length() + 1 + sizeof(unsigned int) + length);
		pw.writeValue(static_cast<unsigned char>(PT_MESCHANNELPEER));
		pw.writeValue(id);
		pw.writeValue(subChannel);
		pw.writeString(currentChannelName.c_str());
		// Copy char array
		if (!pw.copyChars(pr.getChars(length), length)) return ERROR_PACKET;

		sendPacket(0, pw.getArray(), pw.getLength(), getPeerFromID(targetID));
		break;
	}

	case PT_NAME:
	{
		// Request name
		// Check packet
		unsigned int id = 0;
		if (!pr.getValue(id)) return ERROR_PACKET;
		std::string name;
		if (!pr.getString(name)) return ERROR_PACKET;
		std::string oldname;

		// Check if name exists among all peers
		ENetPeer* currentPeer;
		bool foundName = false;
		std::list<ENetPeer*>::iterator it;
		for (it = enetPeerList.begin(); it != enetPeerList.end(); it++)
		{
			currentPeer = *it;
			oldname = getPVS_Peer(currentPeer)->name;
			if (oldname == name)
			{
				// Name already exists
				foundName = true;
				break;
			}
		}
		if (foundName)
		{
			packetWriter pw(sizeof(unsigned char) + sizeof(char));
			pw.writeValue(static_cast<unsigned char>(PT_NAME));
			pw.writeValue(static_cast<char>(0));
			sendPacket(0, pw.getArray(), pw.getLength(), event.peer);
		}
		else
		{
			// Name not found
			// Find peer and set name
			currentPeer = getPeerFromID(id);
			oldname = getPVS_Peer(currentPeer)->name;
			getPVS_Peer(currentPeer)->oldName = oldname;
			getPVS_Peer(currentPeer)->name = name;
			// Send packet
			packetWriter pw(sizeof(unsigned char) + 1024);
			pw.writeValue(static_cast<unsigned char>(PT_NAME));
			pw.writeValue(static_cast<char>(1));
			pw.writeString(name.c_str());
			pw.writeString(oldname.c_str());
			sendPacket(0, pw.getArray(), pw.getLength(), event.peer);
			onNameSet();
			// TODO: inform other peers of name change
		}
		break;
	}
	case PT_REQUESTNEWCHANNEL:
	{
		// Create channel
		// TODO: possible reason to deny channel joining: channel banlist
		unsigned int id = 0;
		if (!pr.getValue(id)) return ERROR_PACKET;
		if (!pr.getString(currentChannelName)) return ERROR_PACKET;
		if (!pr.getString(currentChannelDescription)) return ERROR_PACKET;
		if (!pr.getValue(currentStatus)) return ERROR_PACKET;
		currentPVS_Peer = getPVS_Peer(event.peer);
		char lock = 0;
		if (!pr.getValue(lock)) return ERROR_PACKET;
		char autodestroy = 1;
		if (!pr.getValue(autodestroy)) return ERROR_PACKET;

		// Check if peer has set name
		if (!currentPVS_Peer->name.empty())
		{
			// Create channel if it doesn't exist
			if (channelManager.createNewChannel(currentChannelName, currentChannelDescription, lock, autodestroy))
			{
				// Send everyone channel creation message
				packetWriter pw(sizeof(unsigned char) + 1 + currentChannelName.length() + 1 + currentChannelDescription.length() + 1);
				pw.writeValue(static_cast<unsigned char>(PT_NEWCHANNEL));
				pw.writeValue(static_cast<unsigned char>(1));
				pw.writeString(currentChannelName.c_str());
				pw.writeString(currentChannelDescription.c_str());
				// Send to all peers in server
				for (auto& it : enetPeerList)
				{
					sendPacket(0, pw.getArray(), pw.getLength(), it);
				}
			}
			// Join it
			if (!channelManager.peerExistsInChannel(currentChannelName, currentPVS_Peer))
			{
				channelManager.addPeer(currentChannelName, currentPVS_Peer, currentStatus);
			}
			else
			{
				// Already connected - ignore request and return
				return ERROR_OTHER;
			}
			PVS_Channel* currentChannel = channelManager.getChannel(currentChannelName);
			peerList* pl = currentChannel->peers;
			unsigned int Npeers = static_cast<unsigned int>(pl->size());
			{
				// Send list of peers back (includes self)
				packetWriter pw(sizeof(unsigned char) + 1 + currentChannelName.length() + 1 + currentChannelDescription.length() + 1 + sizeof(unsigned int) + Npeers * sizeof(unsigned int) + channelManager.getChannel(currentChannelName)->getPeerListNameLength() + Npeers * sizeof(unsigned char));
				pw.writeValue(static_cast<unsigned char>(PT_REQUESTJOINCHANNEL));
				pw.writeValue(static_cast<char>(1));
				pw.writeString(currentChannelName.c_str());
				pw.writeString(currentChannelDescription.c_str());
				pw.writeValue(Npeers);
				peerList::iterator it;
				for (it = pl->begin(); it != pl->end(); it++)
				{
					pw.writeValue((*it)->id);
					pw.writeString((*it)->name.c_str());
					pw.writeValue(currentChannel->status[(*it)->id]);
				}
				sendPacket(0, pw.getArray(), pw.getLength(), event.peer);
				onChannelJoin();
			}

			{
				// Send other peers a connect message
				packetWriter pw(sizeof(unsigned char) + 1 + currentChannelName.length() + 1 + sizeof(unsigned int) + currentPVS_Peer->name.length() + 1 + sizeof(unsigned char));
				pw.writeValue(static_cast<unsigned char>(PT_PEERJOINCHANNEL));
				pw.writeValue(static_cast<char>(1));
				pw.writeString(currentChannelName.c_str());
				pw.writeValue(currentPVS_Peer->id);
				pw.writeString(currentPVS_Peer->name);
				pw.writeValue(currentChannel->status[currentPVS_Peer->id]);
				peerList::iterator it;
				for (it = pl->begin(); it != pl->end(); it++)
				{
					if ((*it)->id == id) // Not self
						continue;
					sendPacket(0, pw.getArray(), pw.getLength(), (*it)->enetpeer);
				}
			}
		}
		else
		{
			// Send fail back
			packetWriter pw(sizeof(unsigned char) + 1);
			pw.writeValue(static_cast<unsigned char>(PT_REQUESTJOINCHANNEL));
			pw.writeValue(static_cast<char>(0));
			sendPacket(0, pw.getArray(), pw.getLength(), event.peer);
			onChannelDenied();
		}
		break;
	}
	case PT_REQUESTJOINCHANNEL:
	{
		// Join channel (WARNING: almost the same as creating channel)
		// TODO: possible reason to deny channel joining: channel banlist
		unsigned int id = 0;
		if (!pr.getValue(id)) return ERROR_PACKET;
		if (!pr.getString(currentChannelName)) return ERROR_PACKET;
		if (!pr.getValue(currentStatus)) return ERROR_PACKET;
		currentPVS_Peer = getPVS_Peer(event.peer);

		// Check if peer has set name
		if (!currentPVS_Peer->name.empty())
		{
			if (!channelManager.channelExists(currentChannelName))
			{
				// Channel does not exist
				packetWriter pw(sizeof(unsigned char) + 1);
				pw.writeValue(static_cast<unsigned char>((PT_REQUESTJOINCHANNEL)));
				pw.writeValue(static_cast<char>(0));
				sendPacket(0, pw.getArray(), pw.getLength(), event.peer);
				onChannelDenied();
				return ERROR_OTHER;
			}
			// Join it
			if (!channelManager.peerExistsInChannel(currentChannelName, currentPVS_Peer))
			{
				channelManager.addPeer(currentChannelName, currentPVS_Peer, currentStatus);
			}
			else
			{
				// Already connected - ignore request and return
				return ERROR_OTHER;
			}
			PVS_Channel* currentChannel = channelManager.getChannel(currentChannelName);
			peerList* pl = currentChannel->peers;
			unsigned int Npeers = static_cast<unsigned int>(pl->size());
			{
				// Send list of peers back (includes self)
				packetWriter pw(sizeof(unsigned char) + 1 + currentChannelName.length() + 1 + currentChannelDescription.length() + 1 + sizeof(unsigned int) + Npeers * sizeof(unsigned int) + channelManager.getChannel(currentChannelName)->getPeerListNameLength() + Npeers * sizeof(unsigned char));
				pw.writeValue(static_cast<unsigned char>(PT_REQUESTJOINCHANNEL));
				pw.writeValue(static_cast<char>(1));
				pw.writeString(currentChannelName.c_str());
				pw.writeString(currentChannelDescription.c_str());
				pw.writeValue(Npeers);
				peerList::iterator it;
				for (it = pl->begin(); it != pl->end(); it++)
				{
					pw.writeValue((*it)->id);
					pw.writeString((*it)->name.c_str());
					pw.writeValue(currentChannel->status[(*it)->id]);

				}
				sendPacket(0, pw.getArray(), pw.getLength(), event.peer);
				onChannelJoin();
			}

			{
				// Send other peers a connect message
				packetWriter pw(sizeof(unsigned char) + 1 + currentChannelName.length() + 1 + sizeof(unsigned int) + currentPVS_Peer->name.length() + 1 + sizeof(unsigned char));
				pw.writeValue(static_cast<unsigned char>(PT_PEERJOINCHANNEL));
				pw.writeValue(static_cast<char>(1));
				pw.writeString(currentChannelName.c_str());
				pw.writeValue(currentPVS_Peer->id);
				pw.writeString(currentPVS_Peer->name);
				pw.writeValue(currentChannel->status[currentPVS_Peer->id]);
				peerList::iterator it;
				for (it = pl->begin(); it != pl->end(); it++)
				{
					if ((*it)->id == id) // Not self
						continue;
					sendPacket(0, pw.getArray(), pw.getLength(), (*it)->enetpeer);
				}
			}
		}
		else
		{
			// Send fail back
			packetWriter pw(sizeof(unsigned char) + 1);
			pw.writeValue(static_cast<unsigned char>(PT_REQUESTJOINCHANNEL));
			pw.writeValue(static_cast<char>(0));
			sendPacket(0, pw.getArray(), pw.getLength(), event.peer);
			onChannelDenied();
		}
		break;
	}
	case PT_REQUESTLEAVECHANNEL:
	{
		// Remove peer from channel
		unsigned int id = 0;
		if (!pr.getValue(id)) return ERROR_PACKET;
		if (!pr.getString(currentChannelName)) return ERROR_PACKET;

		if (!channelManager.channelExists(currentChannelName))
			return ERROR_OTHER; // Make sure it's not a bogus message
		currentPVS_Peer = getPVS_Peer(event.peer);
		channelManager.removePeer(currentChannelName, currentPVS_Peer);
		// Tell other peers
		removePeerFromChannelMessage(currentChannelName, currentPVS_Peer);
		onChannelLeave();
		break;
	}
	case PT_CHANNELLIST:
	{
		// Request for channellist
		sendChannelList(event.peer);
		break;
	}
	case PT_NEWCHANNEL:
	{
		// Peers do not send through this channel;
		break;
	}
	case PT_CHANGEDESCRIPTION:
	{
		if (!pr.getString(currentChannelName)) return ERROR_PACKET;
		if (!pr.getString(currentChannelDescription)) return ERROR_PACKET;

		if (!channelManager.channelExists(currentChannelName))
		{
			// Channel not found
			packetWriter pw(sizeof(unsigned char) + 1);
			pw.writeValue(static_cast<unsigned char>(PT_CHANGEDESCRIPTION));
			pw.writeValue(static_cast<char>(0));
			sendPacket(0, pw.getArray(), pw.getLength(), event.peer);
			onChannelDenied();
			return ERROR_OTHER;
		}
		else
		{
			PVS_Channel* c = channelManager.getChannel(currentChannelName);
			if (!c->lock)
			{
				// Send everyone change of description
				packetWriter pw(sizeof(unsigned char) + currentChannelName.length() + 1 + currentChannelDescription.length() + 1);
				pw.writeValue(static_cast<unsigned char>(PT_CHANGEDESCRIPTION));
				pw.writeString(currentChannelName.c_str());
				pw.writeString(currentChannelDescription.c_str());
				// Send to all peers in server
				for (auto& it : enetPeerList)
				{
					sendPacket(0, pw.getArray(), pw.getLength(), it);
				}
				currentPVS_Peer = getPVS_Peer(event.peer);
				c->description = currentChannelDescription;
				onChangeDescription();

			}
			else
			{
				// Decription is locked
				packetWriter pw(sizeof(unsigned char) + 1);
				pw.writeValue(static_cast<unsigned char>(PT_CHANGEDESCRIPTION));
				pw.writeValue(static_cast<char>(0));
				sendPacket(0, pw.getArray(), pw.getLength(), event.peer);
				onChannelDenied();
				return ERROR_OTHER;
			}
		}
		break;
	}
	case PT_CHANGESTATUS:
	{
		unsigned int id = 0;
		if (!pr.getValue(id)) return ERROR_PACKET;
		if (!pr.getString(currentChannelName)) return ERROR_PACKET;
		if (!pr.getValue(currentStatus)) return ERROR_PACKET;
		currentPVS_Peer = getPVS_Peer(event.peer);

		if (channelManager.channelExists(currentChannelName))
		{
			// Change status
			PVS_Channel* currentChannel = channelManager.getChannel(currentChannelName);
			peerList* pl = currentChannel->peers;
			unsigned char oldStatus = currentChannel->status[currentPVS_Peer->id];
			// Status is the same: do nothing
			if (oldStatus == currentStatus)
				return ERROR_OTHER;
			// Change
			currentChannel->status[currentPVS_Peer->id] = currentStatus;

			// Send message to everyone in channel
			{
				// Send list of peers back (includes self)
				packetWriter pw(sizeof(unsigned char) + sizeof(unsigned int) + currentChannelName.length() + 1 + sizeof(unsigned char) + sizeof(unsigned char));
				pw.writeValue(static_cast<unsigned char>(PT_CHANGESTATUS));
				pw.writeValue(id);
				pw.writeString(currentChannelName.c_str());
				pw.writeValue(currentStatus);
				pw.writeValue(oldStatus);
				// Send to everyone (including self)
				peerList::iterator it;
				for (it = pl->begin(); it != pl->end(); it++)
				{
					sendPacket(0, pw.getArray(), pw.getLength(), (*it)->enetpeer);
				}
			}
			onChangeStatus();
		}
		break;
	}
	}
	return 0;
}

// Inform peers in a channel of a removal
void PVS_Server::removePeerFromChannelMessage(const std::string& channelname, PVS_Peer* peer)
{
	if (!channelManager.channelExists(channelname))
	{
		// Channel could not be found: that means channel was destroyed
		packetWriter pw(sizeof(unsigned char) + 1 + channelname.length() + 1);
		pw.writeValue(static_cast<unsigned char>(PT_NEWCHANNEL));
		pw.writeValue(static_cast<char>(0));
		pw.writeString(channelname.c_str());

		// Send to all peers in server
		for (auto& it : enetPeerList)
		{
			sendPacket(0, pw.getArray(), pw.getLength(), it);
		}
		return;
	}
	// Peer should be removed from channel by now
	packetWriter pw(sizeof(unsigned char) + 1 + channelname.length() + 1 + sizeof(unsigned int) + peer->name.length() + 1 + sizeof(unsigned char));
	pw.writeValue(static_cast<unsigned char>(PT_PEERJOINCHANNEL));
	pw.writeValue(static_cast<char>(0));
	pw.writeString(channelname.c_str());
	pw.writeValue(peer->id);
	pw.writeString(peer->name);
	pw.writeValue(0); // Peer left, cannot check what his status was
	peerList* pl = channelManager.getChannel(channelname)->peers;
	peerList::iterator it;
	for (it = pl->begin(); it != pl->end(); it++)
	{
		sendPacket(0, pw.getArray(), pw.getLength(), (*it)->enetpeer);
	}
}

ENetPeer* PVS_Server::getPeerFromID(unsigned int id)
{
	ENetPeer* currentPeer;
	std::list<ENetPeer*>::iterator it;
	for (it = enetPeerList.begin(); it != enetPeerList.end(); it++)
	{
		currentPeer = *it;
		unsigned int currentid = static_cast<PVS_Peer*>(currentPeer->data)->id;
		if (currentid == id)
		{
			return currentPeer;
		}
	}
	return nullptr;
}

PVS_Peer* PVS_Server::getPVS_PeerFromID(unsigned int id)
{
	ENetPeer* p = getPeerFromID(id);
	if (p == nullptr)
	{
		sprintf(currentErrorString, "Could not find peer %i", id);
		onError();
		return nullptr;
	}
	return static_cast<PVS_Peer*>(p->data);
}

// For debugging purposes
void PVS_Server::showAllPeers()
{
	ENetPeer* currentPeer;
	std::list<ENetPeer*>::iterator it;
	for (it = enetPeerList.begin(); it != enetPeerList.end(); it++)
	{
		currentPeer = *it;
		unsigned int currentid = static_cast<PVS_Peer*>(currentPeer->data)->id;
		std::string name = static_cast<PVS_Peer*>(currentPeer->data)->name;
		printf("%i - %s \n", currentid, name.c_str());
	}
	fflush(stdout);
}

PVS_Peer* PVS_Server::getPVS_Peer(ENetPeer* p)
{
	return static_cast<PVS_Peer*>(p->data);
}

// Send a message to a peer
void PVS_Server::sendToPeer(unsigned char subchannel, const std::string& mes, unsigned int id)
{
	packetWriter pw(sizeof(unsigned char) + 1 + mes.length() + 1);
	pw.writeValue(static_cast<unsigned char>(PT_MESSERVER));
	pw.writeValue(subchannel);
	pw.writeString(mes.c_str());
	sendPacket(0, pw.getArray(), pw.getLength(), getPeerFromID(id));
}

void PVS_Server::sendChannelList(ENetPeer* peer)
{
	// Get total string size
	unsigned int number = static_cast<unsigned int>(channelManager.globalChannelList.size());
	unsigned int size = 0;
	for (auto& it : channelManager.globalChannelList)
	{
		size += static_cast<unsigned int>(it->name.length() + 1);
		size += static_cast<unsigned int>(it->description.length() + 1);
	}
	packetWriter pw(sizeof(unsigned char) + sizeof(unsigned int) + size);
	pw.writeValue(static_cast<unsigned char>(PT_CHANNELLIST));
	pw.writeValue(number);
	for (auto& it : channelManager.globalChannelList)
	{
		pw.writeString(it->name.c_str());
		pw.writeString(it->description.c_str());
	}
	sendPacket(0, pw.getArray(), pw.getLength(), peer);
}

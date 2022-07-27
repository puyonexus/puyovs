#include "PVS_Client.h"
#include <enet/enet.h>
#include <string.h>

PVS_Client::PVS_Client()
{
	host = nullptr;
	serverPeer = nullptr;
	networkInitialized = false;
	nameSet = false;
	idSet = false;
	connected = false;

	currentChannelName = "";
	currentPVS_Peer = nullptr;
	currentStatus = 0;
	oldStatus = 0;

	nameRequested = false;
	disconnectRequested = false;

	channelManager.useReferences = false;
	address = new ENetAddress;
}

PVS_Client::~PVS_Client()
{
	if (networkInitialized)
		enet_deinitialize();

	delete address;
}

// Returns username of client
std::string PVS_Client::getUserName() const
{
	if (!networkInitialized || !connected || !idSet)
		return "";
	else
		return getPVS_Peer()->name;
}

// Return id of client
unsigned int PVS_Client::getID() const
{
	if (!networkInitialized || !connected || !idSet)
		return 0;
	else
		return getPVS_Peer()->id;
}

bool PVS_Client::initNetwork()
{
	if (enet_initialize() != 0) {
		return false;
	}

	address->port = 2424;
	host = enet_host_create(nullptr, // Create a client host
		1, N_CHANNELS, // Allow only 1 outgoing connection
		0, // Use 57600 / 8 for 56K modem with 56 Kbps downstream bandwidth
		0); // Use 14400 / 8 for 56K modem with 14 Kbps upstream bandwidth

	if (!host) {
		return false;
	}
	networkInitialized = true;
	return true;
}

// Connect to server
bool PVS_Client::requestConnect(const char* serverAddress)
{
	enet_address_set_host(address, serverAddress);
	serverPeer = enet_host_connect(host, address, N_CHANNELS, 0);
	serverPeer->data = nullptr; // Use this as mark that connection is not yet acknowledged
	if (!serverPeer) {
		return false;
	}
	return true;
}

// Returns PVS_Peer representing client
PVS_Peer* PVS_Client::getPVS_Peer() const
{
	return static_cast<PVS_Peer*>(serverPeer->data);
}

// Request name change
void PVS_Client::requestName(const std::string& name)
{
	requestName(name.c_str());
}

// Request name change
void PVS_Client::requestName(const char* name)
{
	if (!networkInitialized || !connected || !idSet)
		return;

	// Length of name is max 512 bytes (including null terminator)
	char namecopy[512];
	strcpy(namecopy, name);
	namecopy[511] = 0;
	packetWriter pw(sizeof(unsigned char) + sizeof(unsigned int) + 512);
	pw.writeValue(static_cast<unsigned char>(PT_NAME));
	pw.writeValue(getID());
	pw.writeString(namecopy);
	sendPacket(0, pw.getArray(), pw.getLength(), serverPeer);
	nameRequested = true;
}

// Waits up to 3 seconds to disconnect
void PVS_Client::requestDisconnect()
{
	if (!networkInitialized || !connected)
		return;

	disconnectRequested = true;

	enet_peer_disconnect(serverPeer, 0);
	ENetEvent event;
	while (enet_host_service(host, &event, 3000) > 0) {
		switch (event.type) {
		case ENET_EVENT_TYPE_RECEIVE:
			enet_packet_destroy(event.packet);
			break;
		case ENET_EVENT_TYPE_DISCONNECT:
			disconnect();
			return;
		default:
			enet_packet_destroy(event.packet);
			break;
		}
	}
	// Force disconnect
	disconnect();
	enet_peer_reset(serverPeer);
}

// Ask for channellist
void PVS_Client::requestChannelList() const
{
	packetWriter pw(sizeof(unsigned char) + sizeof(unsigned int));
	pw.writeValue(static_cast<unsigned char>(PT_CHANNELLIST));
	pw.writeValue(getID());
	sendPacket(0, pw.getArray(), pw.getLength(), serverPeer);
}

// Change a channel description
void PVS_Client::requestChannelDescription(const std::string& channelname, const std::string& description) const
{
	requestChannelDescription(channelname.c_str(), description.c_str());
}

// Change a channel description
void PVS_Client::requestChannelDescription(const char* channelname, const char* description) const
{
	packetWriter pw(sizeof(unsigned char) + strlen(channelname) + 1 + strlen(description) + 1);
	pw.writeValue(static_cast<unsigned char>(PT_CHANGEDESCRIPTION));
	pw.writeString(channelname);
	pw.writeString(description);
	sendPacket(0, pw.getArray(), pw.getLength(), serverPeer);
}

void PVS_Client::sendPacket(int channelNum, char* pack, int len, ENetPeer* peer) const
{
	if (!networkInitialized || !connected)
		return;

	ENetPacket* packet = enet_packet_create(pack, len, ENET_PACKET_FLAG_RELIABLE);
	enet_peer_send(peer, channelNum, packet);
	enet_host_flush(host);
}

void PVS_Client::checkEvent()
{
	ENetEvent event;
	while (enet_host_service(host, &event, 0) > 0) {
		switch (event.type) {
		case ENET_EVENT_TYPE_CONNECT:
			connect(event);
			break;
		case ENET_EVENT_TYPE_RECEIVE:
			receive(event);
			// Done with packet
			enet_packet_destroy(event.packet);
			break;
		case ENET_EVENT_TYPE_DISCONNECT:
			disconnect();
			break;
		default:
			break;
		}
	}
}

// Connected to server
void PVS_Client::connect(ENetEvent& event)
{
	serverPeer = event.peer;
	serverPeer->data = new PVS_Peer;
	idSet = false;
	connected = true;
	// Wait for id
}

// Disconnected from server: clean everything up
void PVS_Client::disconnect()
{
	connected = false;
	idSet = false;
	disconnectRequested = false;
	onDisconnect();
	// Leave all channels
	while (!channelManager.globalChannelList.empty()) {
		leaveChannel(channelManager.globalChannelList.back()->name.c_str(), false);
		break;
	}
	// Clean server peer
	delete getPVS_Peer();
}
void PVS_Client::receive(ENetEvent& event)
{
	// Get first byte
	packetReader pr(event.packet);
	unsigned char type = 0;
	if (!pr.getValue(type))
		return;
	// Process package
	switch (type) {
	case PT_CONNECT: {
		// Get id from server: call onConnect
		if (!pr.getValue(getPVS_Peer()->id))
			return;
		idSet = true;
		onConnect();
		break;
	}
	case PT_MESSERVER: {
		// Get string from server
		if (!pr.getValue(subChannel))
			return;
		if (!pr.getString(currentString))
			return;
		;
		onMessageServer();
		break;
	}
	case PT_MESCHANNEL: {
		// Get string from channel
		unsigned int id = 0;
		if (!pr.getValue(id))
			return;
		if (!pr.getValue(subChannel))
			return;
		if (!pr.getString(currentChannelName))
			return;
		if (!pr.getString(currentString))
			return;

		// Check if connected to channel in the first place
		if (channelManager.channelExists(currentChannelName)) {
			// Find peer
			if ((currentPVS_Peer = channelManager.getPeerInChannel(currentChannelName, id))) {
				currentPVS_PeerName = currentPVS_Peer->name;
				onMessageChannel();
			} else {
				currentPVS_PeerName = "could not find peer";
				onMessageChannel();
			}
		}
		break;
	}
	case PT_MESCHANNELPEER: {
		// Get string from peer in channel
		unsigned int id = 0;
		if (!pr.getValue(id))
			return;
		if (!pr.getValue(subChannel))
			return;
		if (!pr.getString(currentChannelName))
			return;
		if (!pr.getString(currentString))
			return;

		// Check if connected to channel in the first place
		if (channelManager.channelExists(currentChannelName)) {
			// Find peer
			if ((currentPVS_Peer = channelManager.getPeerInChannel(currentChannelName, id))) {
				if (currentPVS_Peer == nullptr)
					return;
				currentPVS_PeerName = currentPVS_Peer->name;
				onMessageChannelPeer();
			} else
				currentPVS_PeerName = "could not find peer";
		}
	}
	case PT_RAWMESCHANNEL: {
		// Get string from channel
		unsigned int id = 0;
		if (!pr.getValue(id))
			return;
		if (!pr.getValue(subChannel))
			return;
		if (!pr.getString(currentChannelName))
			return;
		unsigned int length = 0;
		if (!pr.getValue(length))
			return;
		if (length == 0)
			return;
		// After this comes the char array

		currentPacket = event.packet;

		// Check if connected to channel in the first place
		if (channelManager.channelExists(currentChannelName)) {
			// Find peer
			if ((currentPVS_Peer = channelManager.getPeerInChannel(currentChannelName, id))) {
				currentPVS_PeerName = currentPVS_Peer->name;
				onRawMessageChannel();
			} else {
				currentPVS_PeerName = "could not find peer";
				onRawMessageChannel();
			}
		}
		break;
	}
	case PT_RAWMESCHANNELPEER: {
		// Get string from peer in channel
		unsigned int id = 0;
		if (!pr.getValue(id))
			return;
		if (!pr.getValue(subChannel))
			return;
		if (!pr.getString(currentChannelName))
			return;
		unsigned int length = 0;
		if (!pr.getValue(length))
			return;
		if (length == 0)
			return;
		// After this comes the char array

		currentPacket = event.packet;

		// Check if connected to channel in the first place
		if (channelManager.channelExists(currentChannelName)) {
			// Find peer
			if ((currentPVS_Peer = channelManager.getPeerInChannel(currentChannelName, id))) {
				currentPVS_PeerName = currentPVS_Peer->name;
				onRawMessageChannelPeer();
			} else {
				currentPVS_PeerName = "could not find peer";
			}
		}
	}

	case PT_NAME: {
		// Name request response from server
		char ok = 0;
		if (!pr.getValue(ok))
			return;

		if (ok) {
			pr.getString(getPVS_Peer()->name);
			pr.getString(getPVS_Peer()->oldName);
			onNameSet();
		} else {
			onNameDenied();
		}
		break;
	}
	case PT_REQUESTJOINCHANNEL: {
		// Channel join/creation response
		char ok = 0;
		pr.getValue(ok);
		if (ok) {
			// Channel is created/joined: client recieves a list of peers (includes self)
			if (!pr.getString(currentChannelName))
				return;
			if (!pr.getString(currentChannelDescription))
				return;
			// Create channel (==joining channel on client side)
			if (!channelManager.channelExists(currentChannelName))
				channelManager.createNewChannel(currentChannelName, currentChannelDescription, false, true);
			else {
				errorString = "ERROR: Attempting to join a channel you're already connected to.";
				onError();
				return; // This shouldnt happen, attempting to join existing channel
			}
			int Npeers = 0;
			pr.getValue(Npeers);
			for (int i = 0; i < Npeers; i++) {
				// Client creates peers in channel
				unsigned char status;
				PVS_Peer* p = new PVS_Peer();
				if (!pr.getValue(p->id))
					break;
				if (!pr.getString(p->name))
					break;
				if (!pr.getValue(status))
					break;
				channelManager.addPeer(currentChannelName, p, status);
			}
			onChannelJoined();
		} else {
			onChannelDenied();
		}
		break;
	}
	case PT_PEERJOINCHANNEL: {
		// A peer joins/leaves channel
		char join = 0;
		if (!pr.getValue(join))
			return;
		if (!pr.getString(currentChannelName))
			return;
		unsigned int id = 0;
		if (!pr.getValue(id))
			return;
		if (!pr.getString(currentPVS_PeerName))
			return;
		if (channelManager.channelExists(currentChannelName)) {
			if (join) {
				PVS_Peer* newpeer = new PVS_Peer;
				newpeer->id = id;
				newpeer->name = currentPVS_PeerName;
				channelManager.addPeer(currentChannelName, newpeer, currentStatus);
				currentPVS_Peer = newpeer;
				onPeerJoinedChannel();
			} else {
				currentPVS_Peer = channelManager.getPeerInChannel(currentChannelName, currentPVS_PeerName);
				onPeerLeftChannel();
				channelManager.removePeer(currentChannelName, currentPVS_Peer);
			}
		}
		break;
	}
	case PT_CHANNELLIST: {
		// Receive channellist
		unsigned int channels = 0;
		if (!pr.getValue(channels))
			return;
		std::vector<std::string> names;
		std::vector<std::string> descriptions;
		for (unsigned int i = 0; i < channels; i++) {
			std::string str;
			std::string descr;
			if (!pr.getString(str))
				return;
			if (!pr.getString(descr))
				return;
			names.push_back(str);
			descriptions.push_back(descr);
		}
		onGetChannelList(&names, &descriptions);
		break;
	}
	case PT_NEWCHANNEL: {
		// Channel was created/destroyed
		unsigned char create = 2;
		if (!pr.getValue(create))
			return;
		if (create == 0) {
			if (!pr.getString(currentChannelName))
				return;
			onChannelDestroyed();
		} else if (create == 1) {
			if (!pr.getString(currentChannelName))
				return;
			if (!pr.getString(currentChannelDescription))
				return;
			onChannelCreated();
		} else {
			errorString = "Wrong specifier for channel creation";
			onError();
		}
		break;
	}
	case PT_CHANGEDESCRIPTION: {
		// Channel changed description
		if (!pr.getString(currentChannelName))
			return;
		if (!pr.getString(currentChannelDescription))
			return;
		onChannelDescription();
	}
	case PT_CHANGESTATUS: { // Status got changed
		unsigned int id = 0;
		if (!pr.getValue(id))
			return;
		if (id == 0)
			return;
		if (!pr.getString(currentChannelName))
			return;
		if (!pr.getValue(currentStatus))
			return;
		if (!pr.getValue(oldStatus))
			return;

		if (channelManager.channelExists(currentChannelName)) {
			// Find peer
			currentPVS_Peer = channelManager.getPeerInChannel(currentChannelName, id);
			if (currentPVS_Peer == nullptr)
				return;
			currentPVS_PeerName = currentPVS_Peer->name;
			// Set status
			channelManager.setStatus(currentChannelName, currentPVS_Peer, currentStatus);
			onPeerStatus();
		}
	}
	}
}

// Request to create channel
void PVS_Client::createChannel(const std::string& name, const std::string& description, bool lock, bool autodestroy, unsigned char status) const
{
	createChannel(name.c_str(), description.c_str(), lock, autodestroy, status);
}

// Request to create channel
void PVS_Client::createChannel(const char* name, const char* description, bool lock, bool autodestroy, unsigned char status) const
{
	if (!networkInitialized || !connected || !idSet)
		return;

	// A channel in the client holds a container of peers
	// Which should be deleted when leaving the channel

	packetWriter pw(sizeof(unsigned char) + sizeof(unsigned int) + strlen(name) + 1 + strlen(description) + 1 + sizeof(unsigned char) + sizeof(char) + sizeof(char));
	pw.writeValue(static_cast<unsigned char>(PT_REQUESTNEWCHANNEL));
	pw.writeValue(getID());
	pw.writeString(name);
	pw.writeString(description);
	pw.writeValue(status);
	lock ? pw.writeValue(static_cast<char>(1)) : pw.writeValue(static_cast<char>(0));
	autodestroy ? pw.writeValue(static_cast<char>(1)) : pw.writeValue(static_cast<char>(0));

	sendPacket(0, pw.getArray(), pw.getLength(), serverPeer);
}

// Request to join channel
void PVS_Client::joinChannel(const std::string& name, unsigned char status) const
{
	joinChannel(name.c_str(), status);
}

// Request to join channel
void PVS_Client::joinChannel(const char* name, unsigned char status) const
{
	if (!networkInitialized || !connected || !idSet)
		return;
	// A channel in the client holds a container of peers
	// Which should be deleted when leaving the channel

	packetWriter pw(sizeof(unsigned char) + sizeof(unsigned int) + strlen(name) + 1 + sizeof(unsigned char));
	pw.writeValue(static_cast<unsigned char>(PT_REQUESTJOINCHANNEL));
	pw.writeValue(getID());
	pw.writeString(name);
	pw.writeValue(status);
	sendPacket(0, pw.getArray(), pw.getLength(), serverPeer);
}

// Leave channel
void PVS_Client::leaveChannel(const std::string& name, bool send)
{
	if (!channelManager.channelExists(name))
		return;
	leaveChannel(name.c_str(), send);
}

// Leave channel
void PVS_Client::leaveChannel(const char* name, bool send)
{
	if (!channelManager.channelExists(name))
		return;

	channelManager.destroyChannel(name);

	if (!send)
		return;
	packetWriter pw(sizeof(unsigned char) + sizeof(unsigned int) + strlen(name) + 1);
	pw.writeValue(static_cast<unsigned char>(PT_REQUESTLEAVECHANNEL));
	pw.writeValue(getID());
	pw.writeString(name);
	sendPacket(0, pw.getArray(), pw.getLength(), serverPeer);
}

// Send string to server
void PVS_Client::sendToServer(unsigned char subchannel, const std::string& message) const
{
	sendToServer(subChannel, message.c_str());
}

// Send string to server
void PVS_Client::sendToServer(unsigned char subchannel, const char* message) const
{
	packetWriter pw(sizeof(unsigned char) + sizeof(unsigned int) + 1 + strlen(message) + 1);
	pw.writeValue(static_cast<unsigned char>(PT_MESSERVER));
	pw.writeValue(getID());
	pw.writeValue(subchannel);
	pw.writeString(message);
	sendPacket(0, pw.getArray(), pw.getLength(), serverPeer);
}

// Send string to channel
void PVS_Client::sendToChannel(unsigned char subchannel, const std::string& message, const std::string& channelname) const
{
	sendToChannel(subchannel, message.c_str(), channelname.c_str());
}

// Send string to channel
void PVS_Client::sendToChannel(unsigned char subchannel, const char* message, const char* channelname) const
{
	packetWriter pw(sizeof(unsigned char) + sizeof(unsigned int) + 1 + strlen(message) + 1 + strlen(channelname) + 1);
	pw.writeValue(static_cast<unsigned char>(PT_MESCHANNEL));
	pw.writeValue(getID());
	pw.writeValue(subchannel);
	pw.writeString(channelname);
	pw.writeString(message);
	sendPacket(0, pw.getArray(), pw.getLength(), serverPeer);
}

// Send string to peer in channel
void PVS_Client::sendToPeer(unsigned char subchannel, const std::string& message, const std::string& channelname, unsigned int id) const
{
	sendToPeer(subchannel, message.c_str(), channelname.c_str(), id);
}

// Send string to peer in channel
void PVS_Client::sendToPeer(unsigned char subchannel, const char* message, const char* channelname, unsigned int id) const
{
	packetWriter pw(sizeof(unsigned char) + sizeof(unsigned int) + 1 + sizeof(unsigned int) + strlen(message) + 1 + strlen(channelname) + 1);
	pw.writeValue(static_cast<unsigned char>(PT_MESCHANNELPEER));
	pw.writeValue(getID());
	pw.writeValue(subchannel);
	pw.writeValue(id);
	pw.writeString(channelname);
	pw.writeString(message);
	sendPacket(0, pw.getArray(), pw.getLength(), serverPeer);
}

void PVS_Client::changeStatus(const std::string& channelname, unsigned char status) const
{
	changeStatus(channelname.c_str(), status);
}

void PVS_Client::changeStatus(const char* channelname, unsigned char status) const
{
	packetWriter pw(sizeof(unsigned char) + sizeof(unsigned int) + strlen(channelname) + 1 + sizeof(unsigned char));
	pw.writeValue(static_cast<unsigned char>(PT_CHANGESTATUS));
	pw.writeValue(getID());
	pw.writeString(channelname);
	pw.writeValue(status);
	sendPacket(0, pw.getArray(), pw.getLength(), serverPeer);
}

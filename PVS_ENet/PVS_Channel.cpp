#include <enet/enet.h>

#include "PVS_Channel.h"
#include <utility>

PVS_Channel::PVS_Channel(std::string n, std::string d, const bool lockdescriptions, const bool destroy)
	: name(std::move(n))
	, description(std::move(d))
	, lock(lockdescriptions)
	, autoDestroy(destroy)
	, peers(nullptr)
{
	peers = new peerList;
}

PVS_Channel::~PVS_Channel()
{
	delete peers;
}

// Calculate total length of all peernames
unsigned int PVS_Channel::getPeerListNameLength() const
{
	unsigned int length = 0;
	for (const auto& peer : *peers) {
		length += static_cast<unsigned int>(peer->name.length() + 1);
	}
	return length;
}

PVS_ChannelManager::PVS_ChannelManager()
{
	useReferences = true;
}

// Returns channel based on name
PVS_Channel* PVS_ChannelManager::getChannel(const std::string& channelName) const
{
	for (const auto& channel : globalChannelList) {
		if (channel->name == channelName) {
			return channel;
		}
	}
	return nullptr;
}

// Get peerlist from channelname
peerList* PVS_ChannelManager::getPeerList(const std::string& channelName) const
{
	return getChannel(channelName)->peers;
}

// Returns true if channel exists
bool PVS_ChannelManager::channelExists(const std::string& channelName) const
{
	return getChannel(channelName) != nullptr;
}

// Find peer in channel
bool PVS_ChannelManager::peerExistsInChannel(const std::string& channelName, PVS_Peer* peer) const
{
	if (!channelExists(channelName)) {
		return false;
	}

	peerList* p = getPeerList(channelName);

	return std::find(p->begin(), p->end(), peer) != p->end();
}

// Creates a new channel, return true if successful
bool PVS_ChannelManager::createNewChannel(const std::string& channelName, std::string channelDescription, bool lockdescription, bool autodestroy)
{
	if (channelExists(channelName)) {
		return false;
	}

	globalChannelList.push_back(new PVS_Channel(channelName, std::move(channelDescription), lockdescription, autodestroy));
	return true;
}

// Removes channel from the channelList
void PVS_ChannelManager::destroyChannel(const std::string& channelName)
{
	if (!channelExists(channelName)) {
		return;
	}

	// Delete channel from all peers
	PVS_Channel* c = getChannel(channelName);
	peerList* peers = c->peers;
	if (peers != nullptr) {
		while (!peers->empty()) {
			std::list<std::string>& cl = peers->back()->channels;
			cl.erase(remove(cl.begin(), cl.end(), channelName), cl.end());

			// Delete peer
			if (!useReferences) {
				delete peers->back();
			}

			// Remove peer
			peers->pop_back();
		}
	}

	// Delete the channel
	globalChannelList.erase(remove(globalChannelList.begin(), globalChannelList.end(), c), globalChannelList.end());
	delete c;
}

// Add an PVS_Peer to a channel
void PVS_ChannelManager::addPeer(const std::string& channelName, PVS_Peer* peer, unsigned char status) const
{
	if (!channelExists(channelName)) {
		return;
	}

	getPeerList(channelName)->push_back(peer);
	peer->channels.push_back(channelName);

	// Set status to default
	getChannel(channelName)->status[peer->id] = status;
}

// Add an PVS_Peer to a channel
void PVS_ChannelManager::addPeer(const std::string& channelName, ENetPeer* peer, unsigned char status) const
{
	addPeer(channelName, static_cast<PVS_Peer*>(peer->data), status);
}

void PVS_ChannelManager::removePeer(const std::string& channelName, ENetPeer* peer)
{
	removePeer(channelName, static_cast<PVS_Peer*>(peer->data));
}

// Remove PVS_Peer from channel
void PVS_ChannelManager::removePeer(const std::string& channelName, PVS_Peer* peer)
{
	if (!channelExists(channelName)) {
		return;
	}

	// Remove status
	getChannel(channelName)->status.erase(peer->id);

	// Remove channelname from peer
	std::list<std::string>& cl = peer->channels;
	cl.erase(remove(cl.begin(), cl.end(), channelName), cl.end());

	// Remove peer from channel
	peerList& pl = *getPeerList(channelName);
	pl.erase(remove(pl.begin(), pl.end(), peer), pl.end());

	// Delete the peer
	if (!useReferences) {
		delete peer;
	}

	// Check if it was the last peer
	if (pl.empty() && getChannel(channelName)->autoDestroy) {
		destroyChannel(channelName);
	}
}

// Find peer in channel with name, if not found it returns null
PVS_Peer* PVS_ChannelManager::getPeerInChannel(const std::string& channelName, const std::string& peerName) const
{
	if (!channelExists(channelName)) {
		return nullptr;
	}

	const peerList* pl = getChannel(channelName)->peers;
	for (const auto& peer : *pl) {
		if (peerName == peer->name) {
			return peer;
		}
	}
	return nullptr;
}

// Find peer in channel with id number, if not found it returns null
PVS_Peer* PVS_ChannelManager::getPeerInChannel(const std::string& channelName, unsigned int peerID) const
{
	if (!channelExists(channelName)) {
		return nullptr;
	}

	const peerList* pl = getChannel(channelName)->peers;
	for (const auto& peer : *pl) {
		if (peerID == peer->id) {
			return peer;
		}
	}
	return nullptr;
}

// Change channel description, return true on success
bool PVS_ChannelManager::changeDescription(const std::string& channelName, std::string channelDescription) const
{
	if (!channelExists(channelName)) {
		return false;
	}

	PVS_Channel* c = getChannel(channelName);
	if (c->lock) {
		return false;
	}
	c->description = std::move(channelDescription);
	return true;
}

void PVS_ChannelManager::setStatus(const std::string& channelName, PVS_Peer* peer, unsigned char status) const
{
	if (!channelExists(channelName)) {
		return;
	}

	getChannel(channelName)->status[peer->id] = status;
}

int PVS_ChannelManager::getStatus(const std::string& channelName, PVS_Peer* peer) const
{
	if (!channelExists(channelName)) {
		return -1;
	}

	return getChannel(channelName)->status[peer->id];
}

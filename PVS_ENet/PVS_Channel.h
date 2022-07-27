#pragma once

#include <algorithm>
#include <list>
#include <map>
#include <string>

#include "PVS_Peer.h"

enum ENet_channelnum {
	EC_CONNECT,
	EC_MESSERVER,
	EC_MESCHANNEL,
	EC_MESCHANNELPEER,
	EC_NAME,
	EC_CREATECHANNEL,
	EC_JOINCHANNEL,
	EC_PEERLIST,
	EC_CHANNELLIST
};

struct _ENetPeer;

typedef std::list<PVS_Peer*> peerList;

// Structure for an "IRC" like channel
struct PVS_Channel {
	enum PVS_ChannelType {
		CT_CHAT, // Just a chat channel
		CT_GAME, // Use for games
		CT_PRIVATE // Use for pm
	};

	PVS_Channel(std::string n, std::string d, bool lockdescriptions, bool destroy = true);
	~PVS_Channel();
	std::string name;
	std::string description;
	bool lock;
	bool autoDestroy;
	peerList* peers;
	unsigned int getPeerListNameLength() const;
	std::map<unsigned int, unsigned char> status;
};

typedef std::list<PVS_Channel*> channelList;

struct PVS_ChannelManager {
	PVS_ChannelManager();
	channelList globalChannelList;

	// On server side: the PVS_Peers are contained in ENet_peer managed by enet
	// On client side: PVS_Peers are instanced in every PVS_Channel
	bool useReferences;

	// Functions related to the channels list
	PVS_Channel* getChannel(const std::string& channelName) const;
	peerList* getPeerList(const std::string& channelName) const;
	bool channelExists(const std::string& channelName) const;
	PVS_Peer* getPeerInChannel(const std::string& channelName, const std::string& peerName) const;
	PVS_Peer* getPeerInChannel(const std::string& channelName, unsigned int peerID) const;
	bool peerExistsInChannel(const std::string& channelName, PVS_Peer* peer) const;
	bool createNewChannel(const std::string& channelName, std::string channelDescription, bool lockDescription, bool autodestroy = true);
	void destroyChannel(const std::string& channelName);
	void addPeer(const std::string& channelName, PVS_Peer* peer, unsigned char status = 0) const;
	void addPeer(const std::string& channelName, _ENetPeer* peer, unsigned char status = 0) const;
	void removePeer(const std::string& channelName, _ENetPeer* peer);
	void removePeer(const std::string& channelName, PVS_Peer* peer);
	bool changeDescription(const std::string& channelname, std::string channelDescription) const;

	void setStatus(const std::string& channelName, PVS_Peer* peer, unsigned char status) const;
	int getStatus(const std::string& channelName, PVS_Peer* peer) const;
};

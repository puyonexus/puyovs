#pragma once

#include "PVS_Channel.h"
#include "PVS_Packet.h"
#include "PVS_Peer.h"
#include <enet/enet.h>

#include <list>
#include <string>

#define N_CHANNELS 10
#define N_MAXCLIENTS 1000

struct PVS_Server {
	PVS_Server();

	ENetHost* host;
	ENetAddress address;
	unsigned int id_counter;
	std::list<ENetPeer*> enetPeerList;
	PVS_ChannelManager channelManager;

	virtual void onInit() = 0;
	virtual void onNameSet() = 0;
	virtual void onConnect(ENetEvent& event) = 0;
	virtual void onDisconnect(ENetEvent& event) = 0;
	virtual void onReceive() = 0;
	virtual void onChannelJoin() = 0;
	virtual void onChannelDenied() = 0;
	virtual void onChannelLeave() = 0;
	virtual void onMessageServer() = 0;
	virtual void onChangeStatus() = 0;
	virtual void onChangeDescription() = 0;
	virtual void onError() = 0;

	bool initNetwork();
	void checkEvent();
	void sendPacket(int channelNum, char* pack, int len, ENetPeer* peer) const;
	void sendToPeer(unsigned char subchannel, const std::string& mes, unsigned int id);
	void sendChannelList(ENetPeer* peer);
	void showAllPeers(); // For debugging purposes
	ENetPeer* getPeerFromID(unsigned int id);
	PVS_Peer* getPVS_PeerFromID(unsigned int id);
	PVS_Peer* getPVS_Peer(ENetPeer* p);
	void removePeerFromChannelMessage(const std::string& name, PVS_Peer* peer);

	std::string currentChannelName;
	std::string currentChannelDescription;
	PVS_Peer* currentPVS_Peer;
	unsigned char subChannel;
	unsigned char currentStatus;
	std::string currentString;
	char currentErrorString[100];

private:
	void connect(ENetEvent& event);
	void disconnect(ENetEvent& event);
	int receive(ENetEvent& event);
};

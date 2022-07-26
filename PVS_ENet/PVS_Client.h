#pragma once

#define N_CHANNELS 10

#include "PVS_Channel.h"
#include "PVS_Packet.h"
#include "PVS_Peer.h"

#include <list>
#include <string>
#include <vector>

struct _ENetHost;
struct _ENetPeer;
struct _ENetAddress;
struct _ENetPacket;
struct _ENetEvent;

// Client functions
struct PVS_Client {
	PVS_Client();
	virtual ~PVS_Client();
	_ENetHost* host;
	_ENetPeer* serverPeer; // Represents server, but client peer data is also stored here
	_ENetAddress* address;
	PVS_ChannelManager channelManager;
	PVS_Peer* getPVS_Peer() const;

	// State
	bool networkInitialized; // Init success
	bool nameSet; // Name set on server
	bool idSet; // ID set on server
	bool connected; // Connected on server, but id may not be set
	bool nameRequested; // Waiting for name request reply
	bool disconnectRequested;

	// Callback
	virtual void onConnect() = 0;
	virtual void onDisconnect() = 0;
	virtual void onNameSet() = 0;
	virtual void onNameDenied() = 0;
	virtual void onChannelJoined() = 0;
	virtual void onChannelDenied() = 0;
	virtual void onChannelLeft() = 0;
	virtual void onGetPeerList(std::vector<std::string>* v) = 0;
	virtual void onGetChannelList(std::vector<std::string>* names, std::vector<std::string>* descriptions) = 0;
	virtual void onMessageServer() = 0;
	virtual void onMessageChannel() = 0;
	virtual void onMessageChannelPeer() = 0;
	virtual void onRawMessageChannel() = 0;
	virtual void onRawMessageChannelPeer() = 0;
	virtual void onPeerJoinedChannel() = 0;
	virtual void onPeerLeftChannel() = 0;
	virtual void onChannelCreated() = 0;
	virtual void onChannelDestroyed() = 0;
	virtual void onChannelDescription() = 0;
	virtual void onPeerStatus() = 0;
	virtual void onError() = 0;

	std::string getUserName() const;
	unsigned int getID() const;

	bool initNetwork();
	void requestName(const char* name);
	void requestName(const std::string& name);
	void requestDisconnect();
	bool requestConnect(const char* serverAddress);
	void requestChannelList() const;
	void requestChannelDescription(const std::string& channelname, const std::string& description) const;
	void requestChannelDescription(const char* channelname, const char* description) const;
	void joinChannel(const std::string& name, unsigned char status = 0) const;
	void joinChannel(const char* name, unsigned char status = 0) const;
	void createChannel(const std::string& name, const std::string& description, bool lock, bool autodestroy = true, unsigned char status = 0) const;
	void createChannel(const char* name, const char* description, bool lock, bool autodestroy = true, unsigned char status = 0) const;
	void leaveChannel(const char* name, bool send = true);
	void leaveChannel(const std::string& name, bool send = true);
	void sendToServer(unsigned char subchannel, const std::string& message) const;
	void sendToServer(unsigned char subchannel, const char* message) const;
	void sendToChannel(unsigned char subchannel, const std::string& message, const std::string& channelname) const;
	void sendToChannel(unsigned char subchannel, const char* message, const char* channelname) const;
	void sendToPeer(unsigned char subchannel, const std::string& message, const std::string& channelname, unsigned int id) const;
	void sendToPeer(unsigned char subchannel, const char* message, const char* channelname, unsigned int id) const;
	void sendPacket(int channelNum, char* pack, int len, _ENetPeer* peer) const;
	void changeStatus(const std::string& channelname, unsigned char status) const;
	void changeStatus(const char* channelname, unsigned char status) const;
	void checkEvent();

	// Accessable variables
	std::string currentChannelName;
	std::string currentChannelDescription;
	PVS_Peer* currentPVS_Peer;
	std::string currentPVS_PeerName;
	unsigned char subChannel;
	unsigned char currentStatus;
	unsigned char oldStatus;
	std::string currentString;
	std::string errorString;
	_ENetPacket* currentPacket;

protected:
	void connect(_ENetEvent& event);
	void disconnect();
	void receive(_ENetEvent& event);
};

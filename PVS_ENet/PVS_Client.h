#pragma once

#define N_CHANNELS 10

#include "PVS_Channel.h"
#include "PVS_Packet.h"
#include "PVS_Peer.h"

#include <string>
#include <list>
#include <vector>

struct _ENetHost;
struct _ENetPeer;
struct _ENetAddress;
struct _ENetPacket;
struct _ENetEvent;

//client functions
struct PVS_Client
{
    PVS_Client();
    virtual ~PVS_Client();
    struct _ENetHost *host;
    struct _ENetPeer *serverPeer; //represents server, but client peer data is also stored here
    struct _ENetAddress *address;
    PVS_ChannelManager channelManager;
    PVS_Peer* getPVS_Peer();


    //state
    bool networkInitialized; //init success
    bool nameSet; //name set on server
    bool idSet; //id set on server
    bool connected; //connected on server, but id may not be set
    bool nameRequested; //waiting for name request reply
    bool disconnectRequested;

    //callback
    virtual void onConnect()=0;
    virtual void onDisconnect()=0;
    virtual void onNameSet()=0;
    virtual void onNameDenied()=0;
    virtual void onChannelJoined()=0;
    virtual void onChannelDenied()=0;
    virtual void onChannelLeft()=0;
    virtual void onGetPeerList(std::vector<std::string> *v)=0;
    virtual void onGetChannelList(std::vector<std::string> *names,std::vector<std::string> *descriptions)=0;
    virtual void onMessageServer()=0;
    virtual void onMessageChannel()=0;
    virtual void onMessageChannelPeer()=0;
    virtual void onRawMessageChannel()=0;
    virtual void onRawMessageChannelPeer()=0;
    virtual void onPeerJoinedChannel()=0;
    virtual void onPeerLeftChannel()=0;
    virtual void onChannelCreated()=0;
    virtual void onChannelDestroyed()=0;
    virtual void onChannelDescription()=0;
    virtual void onPeerStatus()=0;
    virtual void onError()=0;

    std::string getUserName();
    unsigned int getID();

    bool initNetwork();
    void requestName(const char *name);
    void requestName(std::string name);
    void requestDisconnect();
    bool requestConnect(const char *serverAddress);
    void requestChannelList();
    void requestChannelDescription(std::string channelname,std::string description);
    void requestChannelDescription(const char *channelname,const char *description);
    void joinChannel(std::string name,unsigned char status=0);
    void joinChannel(const char *name,unsigned char status=0);
    void createChannel(std::string name,std::string description,bool lock,bool autodestroy=true,unsigned char status=0);
    void createChannel(const char *name,const char *description,bool lock,bool autodestroy=true,unsigned char status=0);
    void leaveChannel(const char *name,bool send=true);
    void leaveChannel(std::string name,bool send=true);
    void sendToServer(unsigned char subchannel,std::string message);
    void sendToServer(unsigned char subchannel,const char *message);
    void sendToChannel(unsigned char subchannel,std::string message,std::string channelname);
    void sendToChannel(unsigned char subchannel,const char *message,const char *channelname);
    void sendToPeer(unsigned char subchannel,std::string message,std::string channelname,unsigned int id);
    void sendToPeer(unsigned char subchannel,const char *message,const char *channelname,unsigned int id);
    void sendPacket(int channelNum, char *pack,int len,struct _ENetPeer *peer);
    void changeStatus(std::string channelname,unsigned char status);
    void changeStatus(const char *channelname,unsigned char status);
    void checkEvent();

    //accessable variables
    std::string currentChannelName;
    std::string currentChannelDescription;
    PVS_Peer *currentPVS_Peer;
    std::string currentPVS_PeerName;
    unsigned char subChannel;
    unsigned char currentStatus;
    unsigned char oldStatus;
    std::string currentString;
    std::string errorString;
    struct _ENetPacket *currentPacket;

    protected:
    void connect(struct _ENetEvent &event);
    void disconnect();
    void receive(struct _ENetEvent &event);
};

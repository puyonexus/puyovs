#pragma once

#include <map>
#include <list>
#include <string>
#include <algorithm>

#include "PVS_Peer.h"

enum ENet_channelnum
{
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

//struct for an "IRC" like channel
struct PVS_Channel
{
    enum PVS_ChannelType
    {
        CT_CHAT, //just a chat channel
        CT_GAME, //use for games
        CT_PRIVATE //use for pm
    };

    PVS_Channel(std::string n,std::string d,bool lockdescriptions,bool destroy=true);
    ~PVS_Channel();
    std::string name;
    std::string description;
    bool lock;
    bool autoDestroy;
    peerList *peers;
    unsigned int getPeerListNameLength();
    std::map<unsigned int,unsigned char> status;
};

typedef std::list<PVS_Channel*> channelList;

struct PVS_ChannelManager
{
    PVS_ChannelManager();
    channelList globalChannelList;

    //on server side: the PVS_Peers are contained in ENet_peer managed by enet
    //on client side: PVS_Peers are instanced in every PVS_Channel
    bool useReferences;

    //functions related to the channels list
    PVS_Channel* getChannel(std::string channelName);
    peerList* getPeerList(std::string channelName);
    bool channelExists(std::string channelName);
    PVS_Peer* getPeerInChannel(std::string channelName,std::string peerName);
    PVS_Peer* getPeerInChannel(std::string channelName,unsigned int peerID);
    bool peerExistsInChannel(std::string channelName,PVS_Peer *peer);
    bool createNewChannel(std::string channelName,std::string channelDescription,bool lockDescription,bool autodestroy=true);
    void destroyChannel(std::string channelName);
    void addPeer(std::string channelName,PVS_Peer *peer,unsigned char status=0);
    void addPeer(std::string channelName,struct _ENetPeer *peer,unsigned char status=0);
    void removePeer(std::string channelName,struct _ENetPeer *peer);
    void removePeer(std::string channelName,PVS_Peer *peer);
    bool changeDescription(std::string channelname,std::string channelDescription);

    void setStatus(std::string channelName,PVS_Peer *peer,unsigned char status);
    int getStatus(std::string channelName,PVS_Peer *peer);
};

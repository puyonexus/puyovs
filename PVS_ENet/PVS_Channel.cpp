#include <enet/enet.h>
#include "PVS_Channel.h"

PVS_Channel::PVS_Channel(std::string n,std::string d,bool lockdescriptions,bool destroy)
: name(n),description(d),lock(lockdescriptions),autoDestroy(destroy),peers(NULL)
{
    peers=new peerList;
}

PVS_Channel::~PVS_Channel()
{
    delete peers;
}

unsigned int PVS_Channel::getPeerListNameLength()
{//calculate total length of all peernames
    unsigned int length=0;
    peerList::iterator it;
    for(it=peers->begin();it!=peers->end();it++)
    {
        length+=(*it)->name.length()+1;
    }
    return length;
}

PVS_ChannelManager::PVS_ChannelManager()
{
    useReferences=true;
}

PVS_Channel* PVS_ChannelManager::getChannel(std::string channelName)
{//returns channel based on name
    channelList::iterator it;
    for (it=globalChannelList.begin();it!=globalChannelList.end();it++)
    {
        if ((*it)->name.compare(channelName)==0)
            return *it;
    }
    return NULL;
}

peerList* PVS_ChannelManager::getPeerList(std::string channelName)
{//get peerlist from channelname
    return getChannel(channelName)->peers;
}

bool PVS_ChannelManager::channelExists(std::string channelName)
{//returns true if channel exists
    if (getChannel(channelName)==NULL)
        return false;
    else
        return true;
}
bool PVS_ChannelManager::peerExistsInChannel(std::string channelName,PVS_Peer *peer)
{//find peer in channel
    if (!channelExists(channelName))
        return false;

    peerList *p=getPeerList(channelName);
    if (std::find(p->begin(),p->end(),peer)==p->end())
        return false;
    else
        return true;
}

bool PVS_ChannelManager::createNewChannel(std::string channelName,std::string channelDescription,bool lockdescription,bool autodestroy)
{//creates a new channel, return true if successful
    if (channelExists(channelName))
        return false;
    globalChannelList.push_back(new PVS_Channel(channelName,channelDescription,lockdescription,autodestroy));
    return true;
}

void PVS_ChannelManager::destroyChannel(std::string channelName)
{//removes channel from the channelList
    if (!channelExists(channelName))
        return;

    //delete channel from all peers
    PVS_Channel *c=getChannel(channelName);
    peerList *peers=c->peers;
    if (peers!=NULL)
    {
        while(peers->size()>0)
        {
            std::list<std::string> &cl=peers->back()->channels;
            cl.erase(remove(cl.begin(),cl.end(),channelName),cl.end());

            //delete peer
            if (!useReferences)
                delete peers->back();
            //remove peer
            peers->pop_back();
        }
    }

    //delete the channel
    globalChannelList.erase(remove(globalChannelList.begin(),globalChannelList.end(),c),globalChannelList.end());
    delete c;
}

void PVS_ChannelManager::addPeer(std::string channelName,PVS_Peer *peer,unsigned char status)
{//add an PVS_Peer to a channel
    if (!channelExists(channelName))
        return;

    getPeerList(channelName)->push_back(peer);
    peer->channels.push_back(channelName);
    //set status to default
    getChannel(channelName)->status[peer->id]=status;
}

void PVS_ChannelManager::addPeer(std::string channelName,ENetPeer *peer,unsigned char status)
{//add an PVS_Peer to a channel
    addPeer(channelName,static_cast<PVS_Peer*>(peer->data),status);
}

void PVS_ChannelManager::removePeer(std::string channelName,ENetPeer *peer)
{
    removePeer(channelName,static_cast<PVS_Peer*>(peer->data));
}
void PVS_ChannelManager::removePeer(std::string channelName,PVS_Peer *peer)
{//remove PVS_Peer from channel
    if (!channelExists(channelName))
        return;

    //remove status
    getChannel(channelName)->status.erase(peer->id);

    //remove channelname from peer
    std::list<std::string> &cl=peer->channels;
    cl.erase(remove(cl.begin(),cl.end(),channelName),cl.end());

    //remove peer from channel
    peerList &pl=*getPeerList(channelName);
    pl.erase(remove(pl.begin(),pl.end(),peer),pl.end());
    //delete the peer
    if (!useReferences)
        delete peer;

    //check if it was the last peer
    if (pl.empty() && getChannel(channelName)->autoDestroy)
        destroyChannel(channelName);
}

PVS_Peer* PVS_ChannelManager::getPeerInChannel(std::string channelName,std::string peerName)
{//find peer in channel with name, if not found it returns null
    if (!channelExists(channelName))
        return NULL;

    peerList *pl=getChannel(channelName)->peers;
    peerList::iterator it;
    for (it=pl->begin();it!=pl->end();it++)
    {
        if (peerName.compare((*it)->name)==0)
        {
            return (*it);
        }
    }
    return NULL;
}
PVS_Peer* PVS_ChannelManager::getPeerInChannel(std::string channelName,unsigned int peerID)
{//find peer in channel with id number, if not found it returns null
    if (!channelExists(channelName))
        return NULL;

    peerList *pl=getChannel(channelName)->peers;
    peerList::iterator it;
    for (it=pl->begin();it!=pl->end();it++)
    {
        if (peerID==(*it)->id)
        {
            return (*it);
        }
    }
    return NULL;
}
bool PVS_ChannelManager::changeDescription(std::string channelName,std::string channelDescription)
{//change channel description, return true on success
    if(!channelExists(channelName))
        return false;

    PVS_Channel *c=getChannel(channelName);
    if (c->lock)
        return false;
    c->description=channelDescription;
    return true;
}

void PVS_ChannelManager::setStatus(std::string channelName,PVS_Peer *peer,unsigned char status)
{
    if(!channelExists(channelName))
        return;

    getChannel(channelName)->status[peer->id]=status;
}

int PVS_ChannelManager::getStatus(std::string channelName,PVS_Peer *peer)
{
    if(!channelExists(channelName))
        return -1;

    return getChannel(channelName)->status[peer->id];
}

#include "PVS_Server.h"

#define ERROR_PACKET 1
#define ERROR_OTHER 2

PVS_Server::PVS_Server()
{
    host=0;
    id_counter=1;
}

void PVS_Server::checkEvent()
{
    ENetEvent event;
    // processing incoming events:
    while (enet_host_service (host, &event, 1) > 0)
    {
        switch (event.type)
        {
        case ENET_EVENT_TYPE_CONNECT:
            connect(event);
            break;
        case ENET_EVENT_TYPE_RECEIVE:
            if (receive(event)==0)
                onReceive();
            else if (receive(event)==ERROR_PACKET)
            {
                sprintf(currentErrorString,"Error at reading or writing packet.");
                onError();
            }
            else if (receive(event)==ERROR_OTHER)
            {
                sprintf(currentErrorString,"Bad or strange request from peer.");
                onError();
            }
            enet_packet_destroy (event.packet);
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
    if (enet_initialize () != 0)
        return false;
    atexit (enet_deinitialize);

    /* Bind the server to the default localhost.     */
    /* A specific host address can be specified by   */
    /* enet_address_set_host (& address, "x.x.x.x"); */

    address.port = 2424;
    address.host = ENET_HOST_ANY;
    host = enet_host_create (&address, // the address to bind the server host to
                             N_MAXCLIENTS,N_CHANNELS, //number of clients
                             0,   // assume any amount of incoming bandwidth
                             0);  // assume any amount of outgoing bandwidth
    if (!host)
        return false;

    onInit();
    return true;
}

void PVS_Server::sendPacket(int channelNum,char *pack,int len,ENetPeer *peer)
{
    /* Create a reliable packet of size 7 containing "packet\0" */
    ENetPacket *packet=enet_packet_create(pack,len,ENET_PACKET_FLAG_RELIABLE);

    /* Send the packet to the peer over channel id 0. */
    /* One could also broadcast the packet by         */
    /* enet_host_broadcast (host, 0, packet);         */
    enet_peer_send(peer, channelNum, packet);
    enet_host_flush(host);
}

void PVS_Server::connect(ENetEvent &event)
{//A peer connected
    //add to list
    enetPeerList.push_back(event.peer);
    //initialize data
    event.peer->data=new PVS_Peer;
    currentPVS_Peer=static_cast<PVS_Peer*>(event.peer->data);
    currentPVS_Peer->id=id_counter;
    currentPVS_Peer->enetpeer=event.peer;

    //send connected confirmation
    packetWriter pw(sizeof(unsigned char)+sizeof(unsigned int));
    pw.writeValue((unsigned char)PT_CONNECT);
    pw.writeValue(id_counter);
    sendPacket(0,pw.getArray(),pw.getLength(),event.peer);
    //
    id_counter++;
    onConnect(event);
    //send a channellist
    sendChannelList(event.peer);
}
void PVS_Server::disconnect(ENetEvent &event)
{//A peer disconnected
    currentPVS_Peer=getPVS_Peer(event.peer);
    //callback before doing anything
    onDisconnect(event);
    //remove from channels
    std::list<std::string> &cl=currentPVS_Peer->channels;
    while (cl.size()!=0)
    {
        currentChannelName=*cl.begin();
        channelManager.removePeer(*cl.begin(),currentPVS_Peer);
        removePeerFromChannelMessage(currentChannelName,currentPVS_Peer);
    }
    //delete PVS_Peer
    delete currentPVS_Peer;
    event.peer->data=NULL;
    //remove from global peerlist
    enetPeerList.erase(std::find(enetPeerList.begin(),enetPeerList.end(),event.peer));
}
int PVS_Server::receive(ENetEvent &event)
{
    //process package
    //get first byte
    packetReader pr(event.packet);
    unsigned char type=0;
    if(!pr.getValue(type)) return ERROR_PACKET;
    //printf("%u\n",type);
    switch (type)
    {
        case PT_CONNECT:
        {//use for debugging (the client should never send on the 0 channel)
            //check packet
            std::string mes;
            if (!pr.getString(mes)) return ERROR_PACKET;

            puts(mes.c_str());
            fflush(stdout);
            break;
        }
        case PT_MESSERVER:
        {//Send string to/from server
            //check packet
            unsigned int id=0;
            if (!pr.getValue(id)) return ERROR_PACKET;
            if (!pr.getValue(subChannel)) return ERROR_PACKET;
            if (!pr.getString(currentString)) return ERROR_PACKET;

            //select peer
            currentPVS_Peer=getPVS_PeerFromID(id);
            onMessageServer();
            break;
        }
        case PT_MESCHANNEL:
        {//Send string to/from channel
            //check packet
            unsigned int id=0;
            if (!pr.getValue(id)) return ERROR_PACKET;
            if (!pr.getValue(subChannel)) return ERROR_PACKET;
            std::string channel;
            if (!pr.getString(channel)) return ERROR_PACKET;
            if (!pr.getString(currentString)) return ERROR_PACKET;

            //does channel exist?
            if (!channelManager.channelExists(channel))
                return ERROR_OTHER;
            //check if peer is in channel
            if (!channelManager.peerExistsInChannel(channel,getPVS_PeerFromID(id)))
                return ERROR_OTHER;

            //write a new packet
            packetWriter pw(sizeof(unsigned char)+sizeof(unsigned int)+1+channel.length()+1+currentString.length()+1);
            pw.writeValue((unsigned char)PT_MESCHANNEL);
            pw.writeValue(id);
            pw.writeValue(subChannel);
            pw.writeString(channel.c_str());
            pw.writeString(currentString.c_str());

            //pass it to all peers in channel, except the sender
            peerList *pl=channelManager.getChannel(channel)->peers;
            peerList::iterator it;
            for (it=pl->begin();it!=pl->end();it++)
            {
                if ((*it)->id==id)
                    continue;
                sendPacket(0,pw.getArray(),pw.getLength(),(*it)->enetpeer);
            }
            break;
        }
        case PT_MESCHANNELPEER:
        {//Send string to/from peer
            unsigned int id=0;
            if (!pr.getValue(id)) return ERROR_PACKET;
            if (!pr.getValue(subChannel)) return ERROR_PACKET;
            unsigned int targetID=0;
            if (!pr.getValue(targetID)) return ERROR_PACKET;
            if (!pr.getString(currentChannelName)) return ERROR_PACKET;
            if (!pr.getString(currentString)) return ERROR_PACKET;

            //does channel exist?
            if (!channelManager.channelExists(currentChannelName))
                return ERROR_OTHER;
            //check if peers are in channel
            if (!channelManager.peerExistsInChannel(currentChannelName,getPVS_PeerFromID(id)))
                return ERROR_OTHER;
            if (!channelManager.peerExistsInChannel(currentChannelName,getPVS_PeerFromID(targetID)))
                return  ERROR_OTHER;

            //target and peer are the same
            if (id==targetID) return ERROR_OTHER;

            //write a new packet
            packetWriter pw(sizeof(unsigned char)+sizeof(unsigned int)+1+currentChannelName.length()+1+currentString.length()+1);
            pw.writeValue((unsigned char)PT_MESCHANNELPEER);
            pw.writeValue(id);
            pw.writeValue(subChannel);
            pw.writeString(currentChannelName.c_str());
            pw.writeString(currentString.c_str());

            sendPacket(0,pw.getArray(),pw.getLength(),getPeerFromID(targetID));
            break;
        }
        case PT_RAWMESCHANNEL:
        {//Send raw packet to/from channel
            //check packet
            unsigned int id=0;
            if (!pr.getValue(id)) return ERROR_PACKET;
            if (!pr.getValue(subChannel)) return ERROR_PACKET;
            std::string channel;
            if (!pr.getString(channel)) return ERROR_PACKET;
            unsigned int length=0;
            if (!pr.getValue(length)) return ERROR_PACKET;
            if (length==0) return ERROR_PACKET;

            //does channel exist?
            if (!channelManager.channelExists(channel))
                return ERROR_OTHER;
            //check if peer is in channel
            if (!channelManager.peerExistsInChannel(channel,getPVS_PeerFromID(id)))
                return ERROR_OTHER;

            //write a new packet
            packetWriter pw(sizeof(unsigned char)+sizeof(unsigned int)+1+channel.length()+1+sizeof(unsigned int)+length);
            pw.writeValue((unsigned char)PT_MESCHANNEL);
            pw.writeValue(id);
            pw.writeValue(subChannel);
            pw.writeString(channel.c_str());
            pw.writeValue(length);
            //copy char array
            if (!pw.copyChars(pr.getChars(length),length)) return ERROR_PACKET;

            //pass it to all peers in channel, except the sender
            peerList *pl=channelManager.getChannel(channel)->peers;
            peerList::iterator it;
            for (it=pl->begin();it!=pl->end();it++)
            {
                if ((*it)->id==id)
                    continue;
                sendPacket(0,pw.getArray(),pw.getLength(),(*it)->enetpeer);
            }
            break;
        }
        case PT_RAWMESCHANNELPEER:
        {//Send char array to/from peer
            unsigned int id=0;
            if (!pr.getValue(id)) return ERROR_PACKET;
            if (!pr.getValue(subChannel)) return ERROR_PACKET;
            unsigned int targetID=0;
            if (!pr.getValue(targetID)) return ERROR_PACKET;
            if (!pr.getString(currentChannelName)) return ERROR_PACKET;
            unsigned int length=0;
            if (!pr.getValue(length)) return ERROR_PACKET;
            if (length==0) return ERROR_PACKET;

            //does channel exist?
            if (!channelManager.channelExists(currentChannelName))
                return ERROR_OTHER;
            //check if peer is in channel
            if (!channelManager.peerExistsInChannel(currentChannelName,getPVS_PeerFromID(id)))
                return ERROR_OTHER;


            //write a new packet
            packetWriter pw(sizeof(unsigned char)+sizeof(unsigned int)+1+currentChannelName.length()+1+sizeof(unsigned int)+length);
            pw.writeValue((unsigned char)PT_MESCHANNELPEER);
            pw.writeValue(id);
            pw.writeValue(subChannel);
            pw.writeString(currentChannelName.c_str());
            //copy char array
            if (!pw.copyChars(pr.getChars(length),length)) return ERROR_PACKET;

            sendPacket(0,pw.getArray(),pw.getLength(),getPeerFromID(targetID));
            break;
        }

        case PT_NAME:
        {//request name
            //check packet
            unsigned int id=0;
            if (!pr.getValue(id)) return ERROR_PACKET;
            std::string name;
            if (!pr.getString(name)) return ERROR_PACKET;
            std::string oldname;

            //check if name exists among all peers
            ENetPeer *currentPeer;
            bool foundName=false;
            std::list<ENetPeer*>::iterator it;
            for (it=enetPeerList.begin();it!=enetPeerList.end();it++)
            {
                currentPeer=*it;
                oldname=getPVS_Peer(currentPeer)->name;
                if (oldname==name)
                {//name already exists
                    foundName=true;
                    break;
                }
            }
            if (foundName)
            {
                packetWriter pw(sizeof(unsigned char)+sizeof(char));
                pw.writeValue((unsigned char)PT_NAME);
                pw.writeValue(char(0));
                sendPacket(0,pw.getArray(),pw.getLength(),event.peer);
            }
            else
            {//name not found
                //find peer and set name
                currentPeer=getPeerFromID(id);
                oldname=getPVS_Peer(currentPeer)->name;
                getPVS_Peer(currentPeer)->oldName=oldname;
                getPVS_Peer(currentPeer)->name=name;
                //send packet
                packetWriter pw(sizeof(unsigned char)+1024);
                pw.writeValue((unsigned char)PT_NAME);
                pw.writeValue(char(1));
                pw.writeString(name.c_str());
                pw.writeString(oldname.c_str());
                sendPacket(0,pw.getArray(),pw.getLength(),event.peer);
                onNameSet();
                //TO DO: inform other peers of name change
            }
            break;
        }
        case PT_REQUESTNEWCHANNEL:
        {//create channel
            //to do: possible reason to deny channel joining: channel banlist
            unsigned int id=0;
            if (!pr.getValue(id)) return ERROR_PACKET;
            if (!pr.getString(currentChannelName)) return ERROR_PACKET;
            if (!pr.getString(currentChannelDescription)) return ERROR_PACKET;
            if (!pr.getValue(currentStatus)) return ERROR_PACKET;
            currentPVS_Peer=getPVS_Peer(event.peer);
            char lock=0;
            if (!pr.getValue(lock)) return ERROR_PACKET;
            char autodestroy=1;
            if (!pr.getValue(autodestroy)) return ERROR_PACKET;

            //check if peer has set name
            if (currentPVS_Peer->name!="")
            {
                //create channel if it doesn't exist
                if (channelManager.createNewChannel(currentChannelName,currentChannelDescription,lock,autodestroy))
                {
                    //send everyone channel creation message
                    packetWriter pw(sizeof(unsigned char)+1+currentChannelName.length()+1+currentChannelDescription.length()+1);
                    pw.writeValue((unsigned char)PT_NEWCHANNEL);
                    pw.writeValue((unsigned char)1);
                    pw.writeString(currentChannelName.c_str());
                    pw.writeString(currentChannelDescription.c_str());
                    //send to all peers in server
                    for(std::list<ENetPeer*>::iterator it=enetPeerList.begin();it!=enetPeerList.end();it++)
                    {
                        sendPacket(0,pw.getArray(),pw.getLength(),(*it));
                    }
                }
                //join it
                if (!channelManager.peerExistsInChannel(currentChannelName,currentPVS_Peer))
                    channelManager.addPeer(currentChannelName,currentPVS_Peer,currentStatus);
                else
                //already connected - ignore request and return
                    return ERROR_OTHER;
                //return peerlist
                PVS_Channel *currentChannel=channelManager.getChannel(currentChannelName);
                peerList *pl=currentChannel->peers;
                unsigned int Npeers=pl->size();
                {//send list of peers back (includes self)
                    packetWriter pw(sizeof(unsigned char)+1+currentChannelName.length()+1+currentChannelDescription.length()+1+sizeof(unsigned int)+Npeers*sizeof(unsigned int)+channelManager.getChannel(currentChannelName)->getPeerListNameLength()+Npeers*sizeof(unsigned char));
                    pw.writeValue((unsigned char)PT_REQUESTJOINCHANNEL);
                    pw.writeValue(char(1));
                    pw.writeString(currentChannelName.c_str());
                    pw.writeString(currentChannelDescription.c_str());
                    pw.writeValue(Npeers);
                    peerList::iterator it;
                    for (it=pl->begin();it!=pl->end();it++)
                    {
                        pw.writeValue((*it)->id);
                        pw.writeString((*it)->name.c_str());
                        pw.writeValue(currentChannel->status[(*it)->id]);
                    }
                    sendPacket(0,pw.getArray(),pw.getLength(),event.peer);
                    onChannelJoin();
                }

                {//send other peers a connect message
                    packetWriter pw(sizeof(unsigned char)+1+currentChannelName.length()+1+sizeof(unsigned int)+currentPVS_Peer->name.length()+1+sizeof(unsigned char));
                    pw.writeValue((unsigned char)PT_PEERJOINCHANNEL);
                    pw.writeValue(char(1));
                    pw.writeString(currentChannelName.c_str());
                    pw.writeValue(currentPVS_Peer->id);
                    pw.writeString(currentPVS_Peer->name);
                    pw.writeValue(currentChannel->status[currentPVS_Peer->id]);
                    peerList::iterator it;
                    for (it=pl->begin();it!=pl->end();it++)
                    {
                        if ((*it)->id==id) //not self
                            continue;
                        sendPacket(0,pw.getArray(),pw.getLength(),(*it)->enetpeer);
                    }
                }
            }
            else
            {//send fail back
                packetWriter pw(sizeof(unsigned char)+1);
                pw.writeValue((unsigned char)PT_REQUESTJOINCHANNEL);
                pw.writeValue(char(0));
                sendPacket(0,pw.getArray(),pw.getLength(),event.peer);
                onChannelDenied();
            }
            break;
        }
        case PT_REQUESTJOINCHANNEL:
        {//join channel (WARNING: almost the same as creating channel)
            //to do: possible reason to deny channel joining: channel banlist
            unsigned int id=0;
            if (!pr.getValue(id)) return ERROR_PACKET;
            if (!pr.getString(currentChannelName)) return ERROR_PACKET;
            if (!pr.getValue(currentStatus)) return ERROR_PACKET;
            currentPVS_Peer=getPVS_Peer(event.peer);

            //check if peer has set name
            if (currentPVS_Peer->name!="")
            {
                if (!channelManager.channelExists(currentChannelName))
                {//channel does not exist
                    packetWriter pw(sizeof(unsigned char)+1);
                    pw.writeValue((unsigned char)(PT_REQUESTJOINCHANNEL));
                    pw.writeValue(char(0));
                    sendPacket(0,pw.getArray(),pw.getLength(),event.peer);
                    onChannelDenied();
                    return ERROR_OTHER;
                }
                //join it
                if (!channelManager.peerExistsInChannel(currentChannelName,currentPVS_Peer))
                    channelManager.addPeer(currentChannelName,currentPVS_Peer,currentStatus);
                else
                //already connected - ignore request and return
                    return ERROR_OTHER;
                //return peerlist
                PVS_Channel *currentChannel=channelManager.getChannel(currentChannelName);
                peerList *pl=currentChannel->peers;
                unsigned int Npeers=pl->size();
                {//send list of peers back (includes self)
                    packetWriter pw(sizeof(unsigned char)+1+currentChannelName.length()+1+currentChannelDescription.length()+1+sizeof(unsigned int)+Npeers*sizeof(unsigned int)+channelManager.getChannel(currentChannelName)->getPeerListNameLength()+Npeers*sizeof(unsigned char));
                    pw.writeValue((unsigned char)PT_REQUESTJOINCHANNEL);
                    pw.writeValue(char(1));
                    pw.writeString(currentChannelName.c_str());
                    pw.writeString(currentChannelDescription.c_str());
                    pw.writeValue(Npeers);
                    peerList::iterator it;
                    for (it=pl->begin();it!=pl->end();it++)
                    {
                        pw.writeValue((*it)->id);
                        pw.writeString((*it)->name.c_str());
                        pw.writeValue(currentChannel->status[(*it)->id]);

                    }
                    sendPacket(0,pw.getArray(),pw.getLength(),event.peer);
                    onChannelJoin();
                }

                {//send other peers a connect message
                    packetWriter pw(sizeof(unsigned char)+1+currentChannelName.length()+1+sizeof(unsigned int)+currentPVS_Peer->name.length()+1+sizeof(unsigned char));
                    pw.writeValue((unsigned char)PT_PEERJOINCHANNEL);
                    pw.writeValue(char(1));
                    pw.writeString(currentChannelName.c_str());
                    pw.writeValue(currentPVS_Peer->id);
                    pw.writeString(currentPVS_Peer->name);
                    pw.writeValue(currentChannel->status[currentPVS_Peer->id]);
                    peerList::iterator it;
                    for (it=pl->begin();it!=pl->end();it++)
                    {
                        if ((*it)->id==id) //not self
                            continue;
                        sendPacket(0,pw.getArray(),pw.getLength(),(*it)->enetpeer);
                    }
                }
            }
            else
            {//send fail back
                packetWriter pw(sizeof(unsigned char)+1);
                pw.writeValue((unsigned char)PT_REQUESTJOINCHANNEL);
                pw.writeValue(char(0));
                sendPacket(0,pw.getArray(),pw.getLength(),event.peer);
                onChannelDenied();
            }
            break;
        }
        case PT_REQUESTLEAVECHANNEL:
        {//remove peer from channel
            unsigned int id=0;
            if (!pr.getValue(id)) return ERROR_PACKET;
            if (!pr.getString(currentChannelName)) return ERROR_PACKET;

            if(!channelManager.channelExists(currentChannelName))
                return ERROR_OTHER; //make sure it's not a bogus message
            currentPVS_Peer=getPVS_Peer(event.peer);
            channelManager.removePeer(currentChannelName,currentPVS_Peer);
            //tell other peers
            removePeerFromChannelMessage(currentChannelName,currentPVS_Peer);
            onChannelLeave();
            break;
        }
        case PT_CHANNELLIST:
        {//request for channellist
            sendChannelList(event.peer);
            break;
        }
        case PT_NEWCHANNEL:
        {//peers do not send through this channel;
            break;
        }
        case PT_CHANGEDESCRIPTION:
        {
            if (!pr.getString(currentChannelName)) return ERROR_PACKET;
            if (!pr.getString(currentChannelDescription)) return ERROR_PACKET;

            if (!channelManager.channelExists(currentChannelName))
            {//channel not found
                packetWriter pw(sizeof(unsigned char)+1);
                pw.writeValue((unsigned char)PT_CHANGEDESCRIPTION);
                pw.writeValue(char(0));
                sendPacket(0,pw.getArray(),pw.getLength(),event.peer);
                onChannelDenied();
                return ERROR_OTHER;
            }
            else
            {
                PVS_Channel *c=channelManager.getChannel(currentChannelName);
                if (!c->lock)
                {
                    //send everyone change of description
                    packetWriter pw(sizeof(unsigned char)+currentChannelName.length()+1+currentChannelDescription.length()+1);
                    pw.writeValue((unsigned char)PT_CHANGEDESCRIPTION);
                    pw.writeString(currentChannelName.c_str());
                    pw.writeString(currentChannelDescription.c_str());
                    //send to all peers in server
                    for(std::list<ENetPeer*>::iterator it=enetPeerList.begin();it!=enetPeerList.end();it++)
                    {
                        sendPacket(0,pw.getArray(),pw.getLength(),(*it));
                    }
                    currentPVS_Peer=getPVS_Peer(event.peer);
                    c->description=currentChannelDescription;
                    onChangeDescription();

                }
                else
                {//decription is locked
                    packetWriter pw(sizeof(unsigned char)+1);
                    pw.writeValue((unsigned char)PT_CHANGEDESCRIPTION);
                    pw.writeValue(char(0));
                    sendPacket(0,pw.getArray(),pw.getLength(),event.peer);
                    onChannelDenied();
                    return ERROR_OTHER;
                }
            }
            break;
        }
        case PT_CHANGESTATUS:
        {
            unsigned int id=0;
            if (!pr.getValue(id)) return ERROR_PACKET;
            if (!pr.getString(currentChannelName)) return ERROR_PACKET;
            if (!pr.getValue(currentStatus)) return ERROR_PACKET;
            currentPVS_Peer=getPVS_Peer(event.peer);

            if (channelManager.channelExists(currentChannelName))
            {//change status
                PVS_Channel *currentChannel=channelManager.getChannel(currentChannelName);
                peerList *pl=currentChannel->peers;
                unsigned char oldStatus=currentChannel->status[currentPVS_Peer->id];
                //status is the same: do nothing
                if (oldStatus==currentStatus)
                    return ERROR_OTHER;
                //change
                currentChannel->status[currentPVS_Peer->id]=currentStatus;

                //send message to everyone in channel
                {//send list of peers back (includes self)
                    packetWriter pw(sizeof(unsigned char)+sizeof(unsigned int)+currentChannelName.length()+1+sizeof(unsigned char)+sizeof(unsigned char));
                    pw.writeValue((unsigned char)PT_CHANGESTATUS);
                    pw.writeValue(id);
                    pw.writeString(currentChannelName.c_str());
                    pw.writeValue(currentStatus);
                    pw.writeValue(oldStatus);
                    //send to everyone (including self)
                    peerList::iterator it;
                    for (it=pl->begin();it!=pl->end();it++)
                    {
                        sendPacket(0,pw.getArray(),pw.getLength(),(*it)->enetpeer);
                    }
                }
                onChangeStatus();
            }
            break;
        }
    }
    return 0;
}
void PVS_Server::removePeerFromChannelMessage(std::string channelname,PVS_Peer *peer)
{//inform peers in a channel of a removal
    if (!channelManager.channelExists(channelname))
    {//channel could not be found: that means channel was destroyed
        packetWriter pw(sizeof(unsigned char)+1+channelname.length()+1);
        pw.writeValue((unsigned char)PT_NEWCHANNEL);
        pw.writeValue(char(0));
        pw.writeString(channelname.c_str());
        //send to all peers in server
        for(std::list<ENetPeer*>::iterator it=enetPeerList.begin();it!=enetPeerList.end();it++)
        {
            sendPacket(0,pw.getArray(),pw.getLength(),(*it));
        }
        return;
    }
    //peer should be removed from channel by now
    packetWriter pw(sizeof(unsigned char)+1+channelname.length()+1+sizeof(unsigned int)+peer->name.length()+1+sizeof(unsigned char));
    pw.writeValue((unsigned char)PT_PEERJOINCHANNEL);
    pw.writeValue(char(0));
    pw.writeString(channelname.c_str());
    pw.writeValue(peer->id);
    pw.writeString(peer->name);
    pw.writeValue(0);//peer left, cannot check what his status was
    peerList *pl=channelManager.getChannel(channelname)->peers;
    peerList::iterator it;
    for (it=pl->begin();it!=pl->end();it++)
    {
        sendPacket(0,pw.getArray(),pw.getLength(),(*it)->enetpeer);
    }
}
ENetPeer* PVS_Server::getPeerFromID(unsigned int id)
{
    ENetPeer *currentPeer;
    std::list<ENetPeer*>::iterator it;
    for (it=enetPeerList.begin();it!=enetPeerList.end();it++)
    {
        currentPeer=*it;
        unsigned int currentid=static_cast<PVS_Peer*>(currentPeer->data)->id;
        if (currentid==id)
        {
            return currentPeer;
        }
    }
    return NULL;
}
PVS_Peer* PVS_Server::getPVS_PeerFromID(unsigned int id)
{
    ENetPeer *p=getPeerFromID(id);
    if (p==NULL)
    {
        sprintf(currentErrorString,"Could not find peer %i",id);
        onError();
        return NULL;
    }
    return static_cast<PVS_Peer*>(p->data);
}

void PVS_Server::showAllPeers()
{//for debugging purposes
    ENetPeer *currentPeer;
    std::list<ENetPeer*>::iterator it;
    for (it=enetPeerList.begin();it!=enetPeerList.end();it++)
    {
        currentPeer=*it;
        unsigned int currentid=static_cast<PVS_Peer*>(currentPeer->data)->id;
        std::string name=static_cast<PVS_Peer*>(currentPeer->data)->name;
        printf("%i - %s \n",currentid,name.c_str());
    }
    fflush(stdout);
}

PVS_Peer* PVS_Server::getPVS_Peer(ENetPeer *p)
{
     return static_cast<PVS_Peer*>(p->data);
}

void PVS_Server::sendToPeer(unsigned char subchannel,std::string mes,unsigned int id)
{//send a message to a peer
    packetWriter pw(sizeof(unsigned char)+1+mes.length()+1);
    pw.writeValue((unsigned char)PT_MESSERVER);
    pw.writeValue(subchannel);
    pw.writeString(mes.c_str());
    sendPacket(0,pw.getArray(),pw.getLength(),getPeerFromID(id));
}

void PVS_Server::sendChannelList(ENetPeer *peer)
{
    //get total string size
    unsigned int number=channelManager.globalChannelList.size();
    unsigned int size=0;
    for (channelList::iterator it=channelManager.globalChannelList.begin();it!=channelManager.globalChannelList.end();it++)
    {
        size+=(*it)->name.length()+1;
        size+=(*it)->description.length()+1;
    }
    packetWriter pw(sizeof(unsigned char)+sizeof(unsigned int)+size);
    pw.writeValue((unsigned char)PT_CHANNELLIST);
    pw.writeValue(number);
    for (channelList::iterator it=channelManager.globalChannelList.begin();it!=channelManager.globalChannelList.end();it++)
    {
        pw.writeString((*it)->name.c_str());
        pw.writeString((*it)->description.c_str());
    }
    sendPacket(0,pw.getArray(),pw.getLength(),peer);
}

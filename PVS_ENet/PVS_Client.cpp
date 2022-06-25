#include "PVS_Client.h"
#include <string.h>
#include <enet/enet.h>

PVS_Client::PVS_Client()
{
    host=0;
    serverPeer=0;
    networkInitialized=false;
    nameSet=false;
    idSet=false;
    connected=false;

    currentChannelName="";
    currentPVS_Peer=NULL;
    currentStatus=0;
    oldStatus=0;

    nameRequested=false;
    disconnectRequested=false;

    channelManager.useReferences=false;
    address = new ENetAddress;
}

PVS_Client::~PVS_Client()
{
    if (networkInitialized)
        enet_deinitialize();

    delete address;
}
std::string PVS_Client::getUserName()
{//returns username of client
    if (!networkInitialized || !connected || !idSet)
        return "";
    else
        return getPVS_Peer()->name;
}
unsigned int PVS_Client::getID()
{//return id of client
    if (!networkInitialized || !connected || !idSet)
        return 0;
    else
        return getPVS_Peer()->id;
}

bool PVS_Client::initNetwork()
{
    if (enet_initialize()!=0)
    {
        return false;
    }

    address->port = 2424;
    host= enet_host_create (0, // create a client host
                             1,N_CHANNELS, // allow only 1 outgoing connection
                             0, // use 57600 / 8 for 56K modem with 56 Kbps downstream bandwidth
                             0);// use 14400 / 8 for 56K modem with 14 Kbps upstream bandwidth

    if (!host)
    {
        return false;
    }
    networkInitialized=true;
    return true;
}

bool PVS_Client::requestConnect(const char *serverAddress)
{//connect to server
    enet_address_set_host (address, serverAddress);
    serverPeer = enet_host_connect (host, address, N_CHANNELS,0);
    serverPeer->data=0; // use this as mark that connection is not yet acknowledged
    if (!serverPeer)
    {
        return false;
    }
    return true;
}

PVS_Peer* PVS_Client::getPVS_Peer()
{//returns PVS_Peer representing client
    return static_cast<PVS_Peer*>(serverPeer->data);
}

void PVS_Client::requestName(std::string name)
{//request name change
    requestName(name.c_str());
}
void PVS_Client::requestName(const char *name)
{//request name change
    if (!networkInitialized || !connected || !idSet)
        return;

    //length of name is max 512 bytes (including null terminator)
    char namecopy[512];
    strcpy(namecopy,name);
    namecopy[511]=0;
    packetWriter pw(sizeof(unsigned char)+sizeof(unsigned int)+512);
    pw.writeValue((unsigned char)PT_NAME);
    pw.writeValue(getID());
    pw.writeString(namecopy);
    sendPacket(0,pw.getArray(),pw.getLength(),serverPeer);
    nameRequested=true;
}
void PVS_Client::requestDisconnect()
{//waits up to 3 seconds to disconnect
    if (!networkInitialized || !connected)
        return;

    disconnectRequested=true;

    enet_peer_disconnect(serverPeer,0);
    ENetEvent event;
    while (enet_host_service (host, &event, 3000) > 0)
    {
        switch (event.type)
        {
        case ENET_EVENT_TYPE_RECEIVE:
            enet_packet_destroy (event.packet);
            break;
        case ENET_EVENT_TYPE_DISCONNECT:
            disconnect();
            return;
        default:
            enet_packet_destroy (event.packet);
            break;
        }
    }
    //force disconnect
    disconnect();
    enet_peer_reset (serverPeer);
}

void PVS_Client::requestChannelList()
{//ask for channellist
    packetWriter pw(sizeof(unsigned char)+sizeof(unsigned int));
    pw.writeValue((unsigned char)PT_CHANNELLIST);
    pw.writeValue(getID());
    sendPacket(0,pw.getArray(),pw.getLength(),serverPeer);
}

void PVS_Client::requestChannelDescription(std::string channelname,std::string description)
{//change a channel description
    requestChannelDescription(channelname.c_str(),description.c_str());
}
void PVS_Client::requestChannelDescription(const char *channelname,const char *description)
{//change a channel description
    packetWriter pw(sizeof(unsigned char)+strlen(channelname)+1+strlen(description)+1);
    pw.writeValue((unsigned char)PT_CHANGEDESCRIPTION);
    pw.writeString(channelname);
    pw.writeString(description);
    sendPacket(0,pw.getArray(),pw.getLength(),serverPeer);
}

void PVS_Client::sendPacket(int channelNum, char *pack,int len,ENetPeer *peer)
{
    if (!networkInitialized || !connected)
        return;

    ENetPacket * packet = enet_packet_create (pack,len,ENET_PACKET_FLAG_RELIABLE);
    enet_peer_send (peer, channelNum, packet);
    enet_host_flush (host);
}

void PVS_Client::checkEvent()
{
    ENetEvent event;
    while (enet_host_service (host, &event, 0) > 0)
    {
        switch (event.type)
        {
            case ENET_EVENT_TYPE_CONNECT:
                connect(event);
                break;
            case ENET_EVENT_TYPE_RECEIVE:
                receive(event);
                //done with packet
                enet_packet_destroy (event.packet);
                break;
            case ENET_EVENT_TYPE_DISCONNECT:
                disconnect();
                break;
            default:
                break;
        }
    }

}

void PVS_Client::connect(ENetEvent &event)
{//Connected to server
    serverPeer=event.peer;
    serverPeer->data=new PVS_Peer;
    idSet=false;
    connected=true;
    //wait for id
}

void PVS_Client::disconnect()
{//Disconnected from server: clean everything up
    connected=false;
    idSet=false;
    disconnectRequested=false;
    onDisconnect();
    //leave all channels
    while(!channelManager.globalChannelList.empty())
    {
        leaveChannel(channelManager.globalChannelList.back()->name.c_str(),false);
        break;
    }
    //clean server peer
    delete getPVS_Peer();
}
void PVS_Client::receive(ENetEvent &event)
{
    //get first byte
    packetReader pr(event.packet);
    unsigned char type=0;
    if(!pr.getValue(type)) return;
    //process package
    switch(type)
    {
        case PT_CONNECT:
        {//Get id from server: call onConnect
            if (!pr.getValue(getPVS_Peer()->id)) return;
            idSet=true;
            onConnect();
            break;
        }
        case PT_MESSERVER:
        {//Get string from server
            if (!pr.getValue(subChannel)) return;
            if (!pr.getString(currentString)) return;;
            onMessageServer();
            break;
        }
        case PT_MESCHANNEL:
        {//Get string from channel
            unsigned int id=0;
            if(!pr.getValue(id)) return;
            if(!pr.getValue(subChannel)) return;
            if(!pr.getString(currentChannelName)) return;
            if(!pr.getString(currentString)) return;

            //check if connected to channel in the first place
            if(channelManager.channelExists(currentChannelName))
            {
                //find peer
                if ((currentPVS_Peer=channelManager.getPeerInChannel(currentChannelName,id)))
                {
                    currentPVS_PeerName=currentPVS_Peer->name;
                    onMessageChannel();
                }
                else
                {
                    currentPVS_PeerName="could not find peer";
                    onMessageChannel();
                }
            }
            break;
        }
        case PT_MESCHANNELPEER:
        {//Get string from peer in channel
            unsigned int id=0;
            if (!pr.getValue(id)) return;
            if (!pr.getValue(subChannel)) return;
            if (!pr.getString(currentChannelName)) return;
            if (!pr.getString(currentString)) return;

            //check if connected to channel in the first place
            if(channelManager.channelExists(currentChannelName))
            {
                //find peer
                if ((currentPVS_Peer=channelManager.getPeerInChannel(currentChannelName,id)))
                {
                    if (currentPVS_Peer==NULL) return;
                    currentPVS_PeerName=currentPVS_Peer->name;
                    onMessageChannelPeer();
                }
                else
                    currentPVS_PeerName="could not find peer";
            }
        }
        case PT_RAWMESCHANNEL:
        {//Get string from channel
            unsigned int id=0;
            if(!pr.getValue(id)) return;
            if(!pr.getValue(subChannel)) return;
            if(!pr.getString(currentChannelName)) return;
            unsigned int length=0;
            if(!pr.getValue(length)) return;
            if (length==0) return;
            //after this comes the char array

            currentPacket=event.packet;

            //check if connected to channel in the first place
            if(channelManager.channelExists(currentChannelName))
            {
                //find peer
                if ((currentPVS_Peer=channelManager.getPeerInChannel(currentChannelName,id)))
                {
                    currentPVS_PeerName=currentPVS_Peer->name;
                    onRawMessageChannel();
                }
                else
                {
                    currentPVS_PeerName="could not find peer";
                    onRawMessageChannel();
                }
            }
            break;
        }
        case PT_RAWMESCHANNELPEER:
        {//Get string from peer in channel
            unsigned int id=0;
            if (!pr.getValue(id)) return;
            if (!pr.getValue(subChannel)) return;
            if (!pr.getString(currentChannelName)) return;
            unsigned int length=0;
            if(!pr.getValue(length)) return;
            if (length==0) return;
            //after this comes the char array

            currentPacket=event.packet;

            //check if connected to channel in the first place
            if(channelManager.channelExists(currentChannelName))
            {
                //find peer
                if ((currentPVS_Peer=channelManager.getPeerInChannel(currentChannelName,id)))
                {
                    currentPVS_PeerName=currentPVS_Peer->name;
                    onRawMessageChannelPeer();
                }
                else
                    currentPVS_PeerName="could not find peer";
            }
        }

        case PT_NAME:
        {//name request response from server
            char ok=0;
            if(!pr.getValue(ok)) return;

            if (ok)
            {
                if (!pr.getString(getPVS_Peer()->name));
                if (!pr.getString(getPVS_Peer()->oldName));
                onNameSet();
            }
            else
            {
                onNameDenied();
            }
            break;
        }
        case PT_REQUESTJOINCHANNEL:
        {//channel join/creation response
            char ok=0;
            if (!pr.getValue(ok));
            if (ok)
            {//channel is created/joined: client recieves a list of peers (includes self)
                if (!pr.getString(currentChannelName)) return;
                if (!pr.getString(currentChannelDescription)) return;
                //create channel (==joining channel on client side)
                if (!channelManager.channelExists(currentChannelName))
                    channelManager.createNewChannel(currentChannelName,currentChannelDescription,false,true);
                else
                {
                    errorString="ERROR: Attempting to join a channel you're already connected to.";
                    onError();
                    return; // this shouldnt happen, attempting to join existing channel
                }
                int Npeers=0;
                pr.getValue(Npeers);
                for (int i=0;i<Npeers;i++)
                {//client creates peers in channel
                    unsigned char status;
                    PVS_Peer *p=new PVS_Peer();
                    if (!pr.getValue(p->id)) break;
                    if (!pr.getString(p->name)) break;
                    if (!pr.getValue(status)) break;
                    channelManager.addPeer(currentChannelName,p,status);
                }
                onChannelJoined();
            }
            else
            {
                onChannelDenied();
            }
            break;
        }
        case PT_PEERJOINCHANNEL:
        {//a peer joins/leaves channel
            char join=0;
            if(!pr.getValue(join)) return;
            if(!pr.getString(currentChannelName)) return;
            unsigned int id=0;
            if(!pr.getValue(id)) return;
            if(!pr.getString(currentPVS_PeerName)) return;
            //if(!pr.getValue(currentStatus)) return;
            if(channelManager.channelExists(currentChannelName))
            {
                if (join)
                {
                    PVS_Peer *newpeer=new PVS_Peer;
                    newpeer->id=id;
                    newpeer->name=currentPVS_PeerName;
                    channelManager.addPeer(currentChannelName,newpeer,currentStatus);
                    currentPVS_Peer=newpeer;
                    onPeerJoinedChannel();
                }
                else
                {
                    currentPVS_Peer=channelManager.getPeerInChannel(currentChannelName,currentPVS_PeerName);
                    onPeerLeftChannel();
                    channelManager.removePeer(currentChannelName,currentPVS_Peer);
                }
            }
            break;
        }
        case PT_CHANNELLIST:
        {//receive channellist
            unsigned int channels=0;
            if (!pr.getValue(channels)) return;
            std::vector<std::string> names;
            std::vector<std::string> descriptions;
            for (unsigned int i=0;i<channels;i++)
            {
                std::string str;
                std::string descr;
                if (!pr.getString(str)) return;
                if (!pr.getString(descr)) return;
                names.push_back(str);
                descriptions.push_back(descr);
            }
            onGetChannelList(&names,&descriptions);
            break;
        }
        case PT_NEWCHANNEL:
        {//channel was created/destroyed
            unsigned char create=2;
            if (!pr.getValue(create)) return;
            if(create==0)
            {
                if (!pr.getString(currentChannelName)) return;
                onChannelDestroyed();
            }
            else if (create==1)
            {
                if (!pr.getString(currentChannelName)) return;
                if (!pr.getString(currentChannelDescription)) return;
                onChannelCreated();
            }
            else
            {
                errorString="Wrong specifier for channel creation";
                onError();
            }
            break;
        }
        case PT_CHANGEDESCRIPTION:
        {//channel changed description
            if (!pr.getString(currentChannelName)) return;
            if (!pr.getString(currentChannelDescription)) return;
            onChannelDescription();
        }
        case PT_CHANGESTATUS:
        {//status got changed
            unsigned int id=0;
            if (!pr.getValue(id)) return;
            if (id==0) return;
            if (!pr.getString(currentChannelName)) return;
            if (!pr.getValue(currentStatus)) return;
            if (!pr.getValue(oldStatus)) return;

            if (channelManager.channelExists(currentChannelName))
            {
                //find peer
                currentPVS_Peer=channelManager.getPeerInChannel(currentChannelName,id);
                if (currentPVS_Peer==NULL) return;
                currentPVS_PeerName=currentPVS_Peer->name;
                //set status
                channelManager.setStatus(currentChannelName,currentPVS_Peer,currentStatus);
                onPeerStatus();
            }
        }
    }
}

void PVS_Client::createChannel(std::string name,std::string description,bool lock,bool autodestroy,unsigned char status)
{//request to create channel
    createChannel(name.c_str(),description.c_str(),lock,autodestroy,status);
}
void PVS_Client::createChannel(const char *name,const char *description,bool lock,bool autodestroy,unsigned char status)
{//request to create channel
    if (!networkInitialized || !connected || !idSet)
        return;
    //a channel in the client holds a container of peers
    //which should be deleted when leaving the channel

    packetWriter pw(sizeof(unsigned char)+sizeof(unsigned int)+strlen(name)+1+strlen(description)+1+sizeof(unsigned char)+sizeof(char)+sizeof(char));
    pw.writeValue((unsigned char)PT_REQUESTNEWCHANNEL);
    pw.writeValue(getID());
    pw.writeString(name);
    pw.writeString(description);
    pw.writeValue(status);
    lock ? pw.writeValue(char(1)):pw.writeValue(char(0));
    autodestroy ? pw.writeValue(char(1)):pw.writeValue(char(0));

    sendPacket(0,pw.getArray(),pw.getLength(),serverPeer);
}
void PVS_Client::joinChannel(std::string name,unsigned char status)
{//request to join channel
    joinChannel(name.c_str(),status);
}
void PVS_Client::joinChannel(const char *name,unsigned char status)
{//request to join channel
    if (!networkInitialized || !connected || !idSet)
        return;
    //a channel in the client holds a container of peers
    //which should be deleted when leaving the channel

    packetWriter pw(sizeof(unsigned char)+sizeof(unsigned int)+strlen(name)+1+sizeof(unsigned char));
    pw.writeValue((unsigned char)PT_REQUESTJOINCHANNEL);
    pw.writeValue(getID());
    pw.writeString(name);
    pw.writeValue(status);
    sendPacket(0,pw.getArray(),pw.getLength(),serverPeer);
}
void PVS_Client::leaveChannel(std::string name,bool send)
{//leave channel
    if (!channelManager.channelExists(name))
        return;
    leaveChannel(name.c_str(),send);
}
void PVS_Client::leaveChannel(const char *name,bool send)
{//leave channel
    if (!channelManager.channelExists(name))
        return;

    channelManager.destroyChannel(name);

    if (!send)
        return;
    packetWriter pw(sizeof(unsigned char)+sizeof(unsigned int)+strlen(name)+1);
    pw.writeValue((unsigned char)PT_REQUESTLEAVECHANNEL);
    pw.writeValue(getID());
    pw.writeString(name);
    sendPacket(0,pw.getArray(),pw.getLength(),serverPeer);
}

void PVS_Client::sendToServer(unsigned char subchannel,std::string message)
{//send string to server
    sendToServer(subChannel,message.c_str());
}
void PVS_Client::sendToServer(unsigned char subchannel,const char *message)
{//send string to server
    packetWriter pw(sizeof(unsigned char)+sizeof(unsigned int)+1+strlen(message)+1);
    pw.writeValue((unsigned char)PT_MESSERVER);
    pw.writeValue(getID());
    pw.writeValue(subchannel);
    pw.writeString(message);
    sendPacket(0,pw.getArray(),pw.getLength(),serverPeer);
}

void PVS_Client::sendToChannel(unsigned char subchannel,std::string message,std::string channelname)
{//send string to channel
    sendToChannel(subchannel,message.c_str(),channelname.c_str());
}
void PVS_Client::sendToChannel(unsigned char subchannel,const char *message,const char *channelname)
{//send string to channel
    packetWriter pw(sizeof(unsigned char)+sizeof(unsigned int)+1+strlen(message)+1+strlen(channelname)+1);
    pw.writeValue((unsigned char)PT_MESCHANNEL);
    pw.writeValue(getID());
    pw.writeValue(subchannel);
    pw.writeString(channelname);
    pw.writeString(message);
    sendPacket(0,pw.getArray(),pw.getLength(),serverPeer);
}

void PVS_Client::sendToPeer(unsigned char subchannel,std::string message,std::string channelname,unsigned int id)
{//send string to peer in channel
    sendToPeer(subchannel, message.c_str(), channelname.c_str(),id);
}
void PVS_Client::sendToPeer(unsigned char subchannel,const char *message,const char *channelname,unsigned int id)
{//send string to peer in channel
    packetWriter pw(sizeof(unsigned char)+sizeof(unsigned int)+1+sizeof(unsigned int)+strlen(message)+1+strlen(channelname)+1);
    pw.writeValue((unsigned char)PT_MESCHANNELPEER);
    pw.writeValue(getID());
    pw.writeValue(subchannel);
    pw.writeValue(id);
    pw.writeString(channelname);
    pw.writeString(message);
    sendPacket(0,pw.getArray(),pw.getLength(),serverPeer);
}

void PVS_Client::changeStatus(std::string channelname,unsigned char status)
{
    changeStatus(channelname.c_str(),status);
}
void PVS_Client::changeStatus(const char *channelname,unsigned char status)
{
    packetWriter pw(sizeof(unsigned char)+sizeof(unsigned int)+strlen(channelname)+1+sizeof(unsigned char));
    pw.writeValue((unsigned char)PT_CHANGESTATUS);
    pw.writeValue(getID());
    pw.writeString(channelname);
    pw.writeValue(status);
    sendPacket(0,pw.getArray(),pw.getLength(),serverPeer);
}


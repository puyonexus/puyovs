#include <enet/enet.h>
#include "PVS_Packet.h"
#include <string.h>

unsigned short pvs_htons(unsigned short s)
{
	return htons(s);
}

unsigned int pvs_htonl(unsigned int l)
{
	return htonl(l);
}

/*
Packets are structured as following, excluding the first byte:

PT_CONNECT:
	(from server)
	*uint32 peer id

PT_MESSSERVER:
	(from client)
	*uint32 id of sender
	*char subchannel
	*string message

	(from server)
	*char subchannel
	*string message

PT_MESCHANNEL:
	(from client)
	*uint32 id of sender
	*char subchannel
	*string channel name
	*string message

	(from server)
	*uint32 id of sender
	*char subchannel
	*string channel name
	*string message

PT_MESCHANNEL_RAW:
	(from client)
	*uint32 id of sender
	*char subchannel
	*string channel name
	*uint32 length of char array
	*char message[array lenght]

	(from server)
	*uint32 id of sender
	*char subchannel
	*string channel name
	*string message
	*uint32 length of char array
	*char message[array length]

PT_MESCHANNELPEER:
	(from client)
	*uint32 id of sender
	*char subchannel
	*uint32 target id
	*string channel name
	*string message

	(from server)
	*uint32 id of sender
	*char subchannel
	*string channel name
	*string message

PT_MESCHANNELPEER_RAW:
	(from client)
	*uint32 id of sender
	*char subchannel
	*uint32 target id
	*string channel name
	*uint32 length
	*char message[length]

	(from server)
	*uint32 id of sender
	*char subchannel
	*string channel name
	*uint32 length
	*char message[length]

PT_NAME:
	(from client)
	*uint32 id of sender
	*string requested name

	(send from server)
	*char name ok(1) or not(0)
	*string requested name
	*string old name

PT_REQUESTNEWCHANNEL:
	(from client)
	*uint32 id of sender
	*string channel name
	*string channel description
	*unsigned char peer status upon entering
	*byte lock
	*byte destroy if empty

	(from server)
	send back through request join channel

PT_REQUESTJOINCHANNEL:
	(from client)
	*uint32 id of sender
	*string channel name
	*unsigned char peer status upon entering

	(send from server) (send to connected peer, to inform of other peers in channel)
	*char join ok(1) or not(0)
	*string channel name
	*string channel description
	*uint32 number of peers in channel
	*uint32 id1
	*string name1
	*unsigned char status1
	*uint32 id2
	*string name2
	*unsigned char status2
	etc.

PT_REQUESTLEAVECHANNEL:
	(from client)
	*uint32 id of sender
	*string channelname

PT_PEERJOINCHANNEL:
	(send from server) (inform leaving/joining single peer)
	*char peer leaves(0)/joins(1)
	*string channel name
	*uint32 id
	*string name
	*unsigned char status

PT_CHANNELLIST:
	(from client)
	*uint32 id of sender

	(send from server)
	*uint32 number of channels
	*string name1
	*string channeldescription
	*string name2
	...

PT_NEWCHANNEL
	(send from server)
	*char destroy channel (0)
	*string channelname

	*char create channel (1)
	*string channelname
	*string channeldescription

PT_CHANGEDESCRIPTION
	(send from server)
	 *string channelname
	 *string channeldescription

	 (send from client)
	 *string channelname
	 *string channeldescription

PT_CHANGESTATUS
	(send from client)
	*uint32 id
	*string channelname
	*unsigned char new status

	(send from server)
	*uint32 id of peer
	*string channelname
	*unsigned char new status
	*unsigned char old status
*/


packetWriter::packetWriter()
	: ppos(0), arraySize(0), initialized(false), dynSize(false)
{}

// Pass char array
packetWriter::packetWriter(char* p)
{
	set(p);
}

// When passing size, so allocation/deallocation of character array self
packetWriter::packetWriter(size_t size)
{
	dynSize = true;
	arraySize = size;
	initialized = true;
	ppos = 0;
	charArray = new char[arraySize];
}
packetWriter::~packetWriter()
{
	if (dynSize)
		delete[] charArray;
}

// Change char array
void packetWriter::set(char* p)
{
	charArray = p;
	initialized = true;
	ppos = 0;
	dynSize = false;
}

bool packetWriter::writeString(const char* str)
{
	if (!initialized)
		return false;

	size_t len = strlen(str) + 1;

	// Passes array size
	if (dynSize && ppos + len > arraySize)
		return false;

	// Copy string to array
	memcpy(charArray + ppos, str, len);
	ppos += len;
	return true;
}

// Copies n bytes
bool packetWriter::copyChars(char* pack, unsigned int n)
{
	if (!initialized)
		return false;
	if (pack == nullptr)
		return false;
	// Passes array size
	if (dynSize && ppos + n > arraySize)
		return false;
	memcpy(charArray + ppos, pack, n);
	ppos += n;
	return true;
}

packetReader::packetReader()
	: ppos(0), initialized(false)
{}

packetReader::packetReader(ENetPacket* p)
{
	set(p);
}

void packetReader::set(ENetPacket* p)
{
	packet = p;
	initialized = true;
	ppos = 0;
}

bool packetReader::getString(std::string& in)
{
	if (!initialized)
		return false;

	if (ppos >= packet->dataLength)
		return false;

	// Check string length, the string in the packet must contain a null terminator
	if (strlen(reinterpret_cast<char*>(packet->data + ppos)) > packet->dataLength - ppos)
		return false;

	// Copy string
	in = (const char*)(packet->data + ppos);
	ppos += in.length() + 1;
	return true;
}

char* packetReader::getChars(unsigned int n)
{
	// Return char pointer and move current position
	if (!initialized)
		return nullptr;

	if (ppos >= packet->dataLength)
		return nullptr;

	// Check array length
	if (ppos + n > packet->dataLength)
		return nullptr;

	// Return pointer to char array
	ppos += n;
	return reinterpret_cast<char*>(packet->data + ppos - n);
}

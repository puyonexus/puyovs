#pragma once

#include <string>

struct _ENetPacket;

unsigned short pvs_htons(unsigned short);
unsigned int pvs_htonl(unsigned int);

#define PT_CONNECT 1
#define PT_MESSERVER 2
#define PT_MESCHANNEL 3
#define PT_MESCHANNELPEER 4
#define PT_RAWMESCHANNEL 5
#define PT_RAWMESCHANNELPEER 6
#define PT_NAME 7
#define PT_REQUESTNEWCHANNEL 8
#define PT_REQUESTJOINCHANNEL 9
#define PT_REQUESTLEAVECHANNEL 10
#define PT_PEERJOINCHANNEL 11
#define PT_PEERLIST 12
#define PT_CHANNELLIST 13
#define PT_NEWCHANNEL 14
#define PT_CHANGEDESCRIPTION 15
#define PT_CHANGESTATUS 16

// packetWriter writes char arrays
// Define packet size in constructor, or pass a char array
class packetWriter {
	size_t ppos;
	size_t arraySize;
	char* charArray;
	bool initialized;
	bool dynSize;

public:
	packetWriter();
	packetWriter(char* p);
	packetWriter(size_t size);
	~packetWriter();
	// Set & get
	void set(char* p);
	char* getArray() const { return charArray; }
	int getLength() const { return static_cast<int>(ppos); }
	// Write value
	template <class T>
	bool writeValue(T in)
	{
		if (!initialized)
			return false;
		// Passes array size
		if (dynSize && ppos + sizeof(T) > arraySize)
			return false;
		// Endianness
		if (sizeof(in) == 2)
			in = static_cast<T>(pvs_htons(in));
		else if (sizeof(in) == 4)
			in = static_cast<T>(pvs_htonl(in));
		// Put in array
		*reinterpret_cast<T*>(charArray + ppos) = in;
		ppos += sizeof(T);
		return true;
	}
	// Write string
	bool writeString(const char* str);
	bool writeString(std::string& str) { return writeString(str.c_str()); }
	// Copy packet
	bool copyChars(char* pack, unsigned int n);
};

// PacketReader helps reading ENetPackets
class packetReader {
public:
	size_t ppos;
	_ENetPacket* packet;
	bool initialized;
	packetReader();
	packetReader(_ENetPacket* p);
	// Set
	void set(_ENetPacket* p);
	void reset() { ppos = 0; }
	// Read
	template <class T>
	bool getValue(T& in)
	{
		// Function to read a value from packet
		struct _T_ENetPacket {
			size_t referenceCount;
			unsigned int flags;
			unsigned char* data;
			size_t dataLength;
		};
		_T_ENetPacket* t_packet = reinterpret_cast<_T_ENetPacket*>(packet);
		// Already beyond packet length
		if (ppos >= t_packet->dataLength)
			return false;
		// Value doesn't fit in packet
		if (ppos + sizeof(T) > t_packet->dataLength)
			return false;

		// Copy to input
		in = *reinterpret_cast<T*>(t_packet->data + ppos);

		// Endianness
		if (sizeof(in) == 2)
			in = static_cast<T>(pvs_htons(in));
		else if (sizeof(in) == 4)
			in = static_cast<T>(pvs_htonl(in));
		ppos += sizeof(T);
		return true;
	}
	bool getString(std::string& in);
	char* getChars(unsigned int n);
};

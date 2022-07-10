#pragma once

#include <string>

struct _ENetPacket;

unsigned short pvs_htons(unsigned short);
unsigned int pvs_htonl(unsigned int);

//enum packetType
#define PT_CONNECT              1
#define PT_MESSERVER            2
#define PT_MESCHANNEL           3
#define PT_MESCHANNELPEER       4
#define PT_RAWMESCHANNEL        5
#define PT_RAWMESCHANNELPEER    6
#define PT_NAME                 7
#define PT_REQUESTNEWCHANNEL    8
#define PT_REQUESTJOINCHANNEL   9
#define PT_REQUESTLEAVECHANNEL  10
#define PT_PEERJOINCHANNEL      11
#define PT_PEERLIST             12
#define PT_CHANNELLIST          13
#define PT_NEWCHANNEL           14
#define PT_CHANGEDESCRIPTION    15
#define PT_CHANGESTATUS         16

//packetWriter writes char arrays
//define packet size in constructor, or pass a char array
class packetWriter
{
    unsigned int ppos;
    unsigned int arraySize;
    char *charArray;
    bool initialized;
    bool dynSize;
public:
    packetWriter();
    packetWriter(char *p);
    packetWriter(int size);
    ~packetWriter();
    //set & get
    void set(char *p);
    char* getArray(){return charArray;}
    int getLength(){return ppos;}
    //write value
    template <class T> bool writeValue (T in)
    {
        if (!initialized)
            return false;
        //passes array size
        if (dynSize && ppos+sizeof(T)>arraySize)
            return false;
        //endianness
        if (sizeof(in)==2)
            in=pvs_htons(in);
        else if (sizeof(in)==4)
            in=pvs_htonl(in);
        //put in array
        *(T *)(charArray+ppos)=in;
        ppos+=sizeof(T);
        return true;
    }
    //write string
    bool writeString(const char *str);
    bool writeString(std::string &str){return writeString(str.c_str());}
    //copy packet
    bool copyChars(char *pack,unsigned int n);
};

//packetReader helps reading ENetPackets
class packetReader
{
public:
    unsigned int ppos;
    struct _ENetPacket *packet;
    bool initialized;
    packetReader();
    packetReader(struct _ENetPacket *p);
    //set
    void set(struct _ENetPacket *p);
    void reset(){ppos=0;}
    //read
    template <class T> bool getValue(T &in)
    {//function to read a value from packet
        struct _T_ENetPacket
        {
           size_t referenceCount;
           unsigned int flags;
           unsigned char *data;
           size_t dataLength;
        };
        _T_ENetPacket *t_packet = (_T_ENetPacket *)(packet);
        //already beyond packet length
        if (ppos >= t_packet->dataLength)
            return false;
        //value doesn't fit in packet
        if (ppos+sizeof(T) > t_packet->dataLength)
            return false;

        //copy to input
        in=*(T *)(t_packet->data+ppos);
        //endianness
        if (sizeof(in)==2)
            in=pvs_htons(in);
        else if (sizeof(in)==4)
            in=pvs_htonl(in);
        ppos+=sizeof(T);
        return true;
    }
    bool getString(std::string &in);
    char* getChars(unsigned int n);
};

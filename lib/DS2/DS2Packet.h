/*
    Copyright (c) 2012 <benemorius@gmail.com>

    Permission is hereby granted, free of charge, to any person
    obtaining a copy of this software and associated documentation
    files (the "Software"), to deal in the Software without
    restriction, including without limitation the rights to use,
    copy, modify, merge, publish, distribute, sublicense, and/or sell
    copies of the Software, and to permit persons to whom the
    Software is furnished to do so, subject to the following
    conditions:

    The above copyright notice and this permission notice shall be
    included in all copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
    EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
    OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
    NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
    HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
    WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
    FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
    OTHER DEALINGS IN THE SOFTWARE.
*/

#ifndef DS2PACKET_H
#define DS2PACKET_H

#include <stdint.h>
#include <cstdio>
#include "DS2Bus.h"

typedef enum
{
	DS2_K,
	DS2_L,
	DS2_BOTH
} DS2BusType;

typedef enum
{
	DS2_8BIT,
	DS2_16BIT
} DS2PacketType;

typedef enum
{
	DS2_PSTATUS_OK,
	DS2_PSTATUS_INVALID,
	DS2_PSTATUS_CHECKSUM_ERROR,
	DS2_PSTATUS_INSUFFICIENT_DATA
} DS2PStatus;

class DS2Packet
{
public:
	DS2Packet(DS2PacketType type = DS2_16BIT);
	DS2Packet(int address, DS2PacketType type = DS2_16BIT);
	DS2Packet(int address, const uint8_t* data, int dataLength, DS2PacketType type = DS2_16BIT);
	DS2Packet(const uint8_t* rawPacket, int length, DS2PacketType type);
	~DS2Packet();
	
	int getAddress() const {return address;}
	uint8_t* getData() const {return dataPtr;}
	int getDataLength() const {return dataLength;}
	uint8_t getChecksum() const {return checksum;}
	int getPacketLength() const {return packetLength;}
	DS2PacketType getType() const {return type;}
	uint8_t* getRawPacket() const {return rawPacketPtr;}
	DS2PStatus getStatus();

	DS2Packet& setAddress(int address);
	DS2Packet& setData(const uint8_t* data, uint16_t length);

	DS2PStatus packetFromRawPacket(const uint8_t* rawPacketData, uint16_t length, DS2PacketType type);
	void printPacket(FILE* file) const;

private:
	void generatePacket();
	uint8_t _checksum() const;

	int address;
	uint16_t packetLength;
	uint16_t dataLength;
	uint8_t* dataPtr;
	uint8_t* rawPacketPtr;
	uint8_t checksum;
	DS2PacketType type;
};

#endif // DS2PACKET_H

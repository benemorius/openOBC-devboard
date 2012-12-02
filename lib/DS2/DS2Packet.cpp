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

#include "DS2.h"
#include <cstring>
#include "debugpretty.h"

DS2Packet::DS2Packet(DS2PacketType type)
{
	dataPtr = NULL;
	rawPacketPtr = NULL;
	this->type = type;
	this->address = 0;
	setData(0, 0);
}

DS2Packet::DS2Packet(int address, DS2PacketType type)
{
	dataPtr = NULL;
	rawPacketPtr = NULL;
	this->type = type;
	this->address = address;
	setData(0, 0);
}

DS2Packet::DS2Packet(int address, const uint8_t* data, int dataLength, DS2PacketType type)
{
	dataPtr = NULL;
	rawPacketPtr = NULL;
	this->type = type;
	this->address = address;
	setData(data, dataLength);
}

DS2Packet::DS2Packet(const uint8_t* rawPacket, int length, DS2PacketType type)
{
	dataPtr = NULL;
	rawPacketPtr = NULL;
	this->type = type;
	packetFromRawPacket(rawPacket, length, type);
}

DS2Packet::~DS2Packet()
{
	delete[] dataPtr;
	delete[] rawPacketPtr;
}

DS2Packet& DS2Packet::setAddress(int address)
{
	this->address = address;
	generatePacket();
	return *this;
}

DS2Packet& DS2Packet::setData(const uint8_t* data, uint16_t length)
{
	if(length)
	{
		delete[] dataPtr;
		dataPtr = new uint8_t[length];
		memcpy(dataPtr, data, length);
	}

	this->dataLength = length;

	if(type == DS2_8BIT)
		this->packetLength = length + 3;
	else if(type == DS2_16BIT)
		this->packetLength = length + 4;
	else
		throw;

	generatePacket();
	return *this;
}

DS2PStatus DS2Packet::packetFromRawPacket(const uint8_t* rawPacketData, uint16_t length, DS2PacketType type)
{
	int16_t dataLength;
	uint8_t checksum;
	this->type = type;
	
	if(type == DS2_8BIT)
	{
// 		DEBUG("8 bit: size: 0x%x at 0x%x: ", length, rawPacketData);
		if(length < 4)
		{
// 			DEBUG("insufficient data for 8 bit packet\r\n");
			return DS2_PSTATUS_INSUFFICIENT_DATA;
		}
		packetLength = rawPacketData[1];
		dataLength = packetLength - 3;
	}
	else if(type == DS2_16BIT)
	{
// 		DEBUG("16 bit: size: 0x%x at 0x%x: ", length, rawPacketData);
		if(rawPacketData[1] != 0x00)
		{
// 			DEBUG("high address byte is not 0x00: %x %x %x %x %x\r\n", rawPacketData[0], rawPacketData[1], rawPacketData[2], rawPacketData[3], rawPacketData[4]);
			return DS2_PSTATUS_INVALID;
		}
		if(length < 5)
		{
// 			DEBUG("insufficient data for 16 bit packet\r\n");
			return DS2_PSTATUS_INSUFFICIENT_DATA;
		}
		packetLength = rawPacketData[2];
		packetLength += rawPacketData[1] << 8;
		dataLength = packetLength - 4;
	}
	else
	{
		throw;
	}

	if(packetLength == 0)
	{
// 		DEBUG("bad packet: zero packet length: %x %x %x %x %x\r\n", rawPacketData[0], rawPacketData[1], rawPacketData[2], rawPacketData[3], rawPacketData[4]);
		return DS2_PSTATUS_INVALID;
	}
	if(packetLength > MAX_PACKET_LENGTH)
	{
// 		DEBUG("bad packet: packet length > 0x%x : %x %x %x %x %x\r\n", MAX_PACKET_LENGTH, rawPacketData[0], rawPacketData[1], rawPacketData[2], rawPacketData[3], rawPacketData[4]);
		return DS2_PSTATUS_INVALID;
	}
	if(dataLength <= 0)
	{
// 		DEBUG("bad packet: zero data length: %x %x %x %x %x\r\n", rawPacketData[0], rawPacketData[1], rawPacketData[2], rawPacketData[3], rawPacketData[4]);
		return DS2_PSTATUS_INVALID;
	}
	if(length < packetLength)
	{
// 		DEBUG("bad packet: insufficient data (want 0x%x)\r\n", packetLength);
		return DS2_PSTATUS_INSUFFICIENT_DATA;
	}

	if(type == DS2_8BIT)
	{
		setData(rawPacketData+2, dataLength);
		checksum = rawPacketData[dataLength + 2];
		
	}
	else if(type == DS2_16BIT)
	{
		setData(rawPacketData+3, dataLength);
		checksum = rawPacketData[dataLength + 3];
	}
	else
	{
		throw;
	}
	
	setAddress(rawPacketData[0]);

	//return true if packet has a valid checksum
	generatePacket();
	if(checksum == this->checksum)
		return DS2_PSTATUS_OK;
	else
		return DS2_PSTATUS_CHECKSUM_ERROR;
}

void DS2Packet::printPacket(FILE* file) const
{
	if(type == DS2_8BIT)
	{
		fprintf(file, "0x%02x 0x%02x ", getAddress(), getPacketLength());
	}
	else if(type == DS2_16BIT)
	{
		fprintf(file, "0x%02x 0x%02x 0x%02x ", getAddress(), getPacketLength() >> 8, getPacketLength() & 0xff);
	}
	else
	{
		throw;
	}
	for(int i = 0; i < dataLength; i++)
	{
		fprintf(file, "0x%02x ", dataPtr[i]);
	}
	fprintf(file, "0x%02x\n", checksum);
}

void DS2Packet::generatePacket()
{
	delete[] rawPacketPtr;
	rawPacketPtr = new uint8_t[getPacketLength()];
	int ptr = 0;
	rawPacketPtr[ptr++] = address;
	if(type == DS2_8BIT)
	{
		rawPacketPtr[ptr++] = packetLength;
	}
	else if(type == DS2_16BIT)
	{
		rawPacketPtr[ptr++] = (packetLength >> 8) & 0xff;
		rawPacketPtr[ptr++] = packetLength & 0xff;
	}
	else //unhandled packet type
	{
		throw;
	}
	memcpy(rawPacketPtr+ptr, dataPtr, dataLength);
	this->checksum = _checksum();
	rawPacketPtr[dataLength+ptr] = this->checksum;
}	

uint8_t DS2Packet::_checksum() const
{
	uint8_t checksum = 0;
	for(int i = 0; i < packetLength - 1; i++)
		checksum ^= rawPacketPtr[i];
	return checksum;
}

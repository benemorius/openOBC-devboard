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
#include <string.h>
#include "delay.h"
#include "debugpretty.h"

using namespace std;

DS2::DS2(DS2Bus& k, DS2Bus& l) : k(k), l(l)
{
	receiveTimeoutK.start();
	receiveTimeoutL.start();
	bufferK = new deque<uint8_t>;
	bufferL = new deque<uint8_t>;
	taskEnabled = false;
	interceptPackets = false;
	k.attach(this, &DS2::receiveHandlerK);
	l.attach(this, &DS2::receiveHandlerL);
}

bool DS2::write(const DS2Packet& txPacket, DS2BusType txBus)
{
	if(txBus == DS2_K || txBus == DS2_BOTH)
	{
// 		DEBUG("writing to k\r\n");
		if(k.write(txPacket.getRawPacket(), txPacket.getPacketLength()) != txPacket.getPacketLength())
			return false;
	}
	if(txBus == DS2_L || txBus == DS2_BOTH)
	{
// 		DEBUG("writing to l\r\n");
		if(l.write(txPacket.getRawPacket(), txPacket.getPacketLength()) != txPacket.getPacketLength())
			return false;
	}
	return true;
}

DS2Packet* DS2::read(DS2BusType bus)
{
	if(((bus == DS2_K) || bus == DS2_BOTH) && !packetBufferK.empty())
	{
		DS2Packet* packet = packetBufferK.front();
		packetBufferK.pop_front();
		return packet;
	}
	if(((bus == DS2_L) || bus == DS2_BOTH) && !packetBufferL.empty())
	{
		DS2Packet* packet = packetBufferL.front();
		packetBufferL.pop_front();
		return packet;
	}
	return NULL;
}

DS2Packet* DS2::query(const DS2Packet& txPacket, DS2BusType txBus, int timeout_ms)
{
	DEBUG("sending: ");
	txPacket.printPacket(stderr);
	fprintf(stderr, "\r");

	interceptPackets = true;
		
	//send the packet and allow time for a reply
	if(!write(txPacket, txBus))
	{
		interceptPackets = false;
		return NULL;
	}

	Timer timeout;
	while(timeout.read_ms() < timeout_ms)
	{
		if(taskEnabled)
			task();
		DS2Packet* rxPacket = read();
		if(rxPacket != NULL)
		{
			DEBUG("received a reply: ");
			rxPacket->printPacket(stderr);
			fprintf(stderr, "\r");
			interceptPackets = false;
			return rxPacket; //TODO maybe verify packet type and address
		}
	}
	interceptPackets = false;
	DEBUG("no reply\r\n");
	return NULL;
}

void DS2::task()
{
	if(!taskEnabled)
		return;
	taskEnabled = false;

	DS2Packet* packet;
	
	//queue packets from k
	while((packet = getPacketFromBus(DS2_K)) != NULL)
	{
		if(callback.isValid() || interceptPackets)
			packetBufferK.push_back(packet);
		else
			delete packet;
	}
	//queue packets from l
	while((packet = getPacketFromBus(DS2_L)) != NULL)
	{
		if(callback.isValid() || interceptPackets)
			packetBufferL.push_back(packet);
		else
			delete packet;
	}
	
	if(callback.isValid() && !interceptPackets && (packetBufferK.size() || packetBufferL.size()))
		callback.call();
}

DS2Packet* DS2::getPacketFromBus(DS2BusType bus) //TODO clean this mess up
{
	DS2Packet* packet;
	bool flushBuffer = false;

	if(bus == DS2_K)
	{
		if(packetBufferK.size() >= MAX_PACKET_COUNT)
		{
// 			DEBUG("too many packets in k: \r\n", packetBufferK.size());
			return NULL;
		}
		if(packetBufferK.size() > 0)
		
		//flush buffer if last byte was received too long ago
		if(bufferK->size() > 0 && receiveTimeoutK.read_ms() > 600)
			flushBuffer = true;
		
		//check for new bytes and add them to the buffer
		if(k.readable() && !flushBuffer)
		{
			uint8_t c;
			while(k.read(&c, 1) == 1)
			{
				bufferK->push_back(c);

				//if the buffer overflows, just drop the whole thing
				if(bufferK->size() >= MAX_PACKET_LENGTH)
				{
// 					DEBUG("ds2 buffer overflow\r\n");
					bufferK->clear();
					break;
				}
			}
			receiveTimeoutK.start();
		}

		//check the buffered data for a packet
		packet = new DS2Packet;
		while(1)
		{
			if(bufferK->size() <= 0)
			{
				delete packet;
				return NULL;
			}
			
			//read the buffered data into a contiguous array
			uint16_t bufferLength = bufferK->size();
			uint8_t* rawPacketBuffer = new uint8_t[bufferLength];
			for(int i = 0; i < bufferLength; i++)
			{
				rawPacketBuffer[i] = bufferK->at(i);
			}
			
			//try packet as both 8 and 16 bit
			DS2PStatus status16;
			DS2PStatus status8;
			if((status16 = packet->packetFromRawPacket(rawPacketBuffer, bufferLength, DS2_16BIT)) == DS2_PSTATUS_OK)
			{
				bufferK->erase(bufferK->begin(), bufferK->begin() + packet->getPacketLength());
				delete rawPacketBuffer;
				return packet;
			}
			else if((status8 = packet->packetFromRawPacket(rawPacketBuffer, bufferLength, DS2_8BIT)) == DS2_PSTATUS_OK)
			{
				bufferK->erase(bufferK->begin(), bufferK->begin() + packet->getPacketLength());
				delete rawPacketBuffer;
				return packet;
			}
			else
			{
				if(flushBuffer || ((status8 != DS2_PSTATUS_INSUFFICIENT_DATA) && (status16 != DS2_PSTATUS_INSUFFICIENT_DATA)))
				{
					bufferK->pop_front();
					if(flushBuffer)
						continue;
				}
			}
			delete rawPacketBuffer;
			break;
		}
		delete packet;
		if(flushBuffer)
			return getPacketFromBus(bus);
		return NULL;

	}
	else if(bus == DS2_L)
	{
		if(packetBufferL.size() >= MAX_PACKET_COUNT)
		{
// 			DEBUG("too many packets in l: \r\n", packetBufferK.size());
			return NULL;
		}
		if(packetBufferL.size() > 0)
			
			//flush buffer if last byte was received too long ago
		if(bufferL->size() > 0 && receiveTimeoutL.read_ms() > 600)
			flushBuffer = true;
		
		//check for new bytes and add them to the buffer
		if(l.readable() && !flushBuffer)
		{
			uint8_t c;
			while(l.read(&c, 1) == 1)
			{
				bufferL->push_back(c);

				//if the buffer overflows, just drop the whole thing
				if(bufferL->size() >= MAX_PACKET_LENGTH)
				{
// 					DEBUG("ds2 buffer overflow\r\n");
					bufferL->clear();
					break;
				}
			}
			receiveTimeoutK.start();
		}
			
		//check the buffered data for a packet
		packet = new DS2Packet;
		while(1)
		{
			if(bufferL->size() <= 0)
			{
				delete packet;
				return NULL;
			}

			//read the buffered data into a contiguous array
			uint16_t bufferLength = bufferL->size();
			uint8_t* rawPacketBuffer = new uint8_t[bufferLength];
			for(int i = 0; i < bufferLength; i++)
			{
				rawPacketBuffer[i] = bufferL->at(i);
			}

			//try packet as both 8 and 16 bit
			DS2PStatus status16;
			DS2PStatus status8;
			if((status16 = packet->packetFromRawPacket(rawPacketBuffer, bufferLength, DS2_16BIT)) == DS2_PSTATUS_OK)
			{
				bufferL->erase(bufferL->begin(), bufferL->begin() + packet->getPacketLength());
				delete rawPacketBuffer;
				return packet;
			}
			else if((status8 = packet->packetFromRawPacket(rawPacketBuffer, bufferLength, DS2_8BIT)) == DS2_PSTATUS_OK)
			{
				bufferL->erase(bufferL->begin(), bufferL->begin() + packet->getPacketLength());
				delete rawPacketBuffer;
				return packet;
			}
			else
			{
				if(flushBuffer || ((status8 != DS2_PSTATUS_INSUFFICIENT_DATA) && (status16 != DS2_PSTATUS_INSUFFICIENT_DATA)))
				{
					bufferL->pop_front();
					if(flushBuffer)
						continue;
				}
			}
			delete rawPacketBuffer;
			break;
		}
		delete packet;
		if(flushBuffer)
			return getPacketFromBus(bus);
		return NULL;
	}
	else
	{
		throw;
	}
}

void DS2::receiveHandlerK()
{
	if(k.readable())
		taskEnabled = true;
}

void DS2::receiveHandlerL()
{
	if(l.readable())
		taskEnabled = true;
}

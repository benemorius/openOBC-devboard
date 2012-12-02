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

#ifndef DS2_H
#define DS2_H

#include <stdint.h>
#include <cstdio>
#include <deque>
#include "DS2Packet.h"
#include "DS2Bus.h"
#include "Timer.h"

#define MAX_PACKET_COUNT (64)
#define MAX_PACKET_LENGTH (RECEIVE_BUFFER_SIZE)

#if MAX_PACKET_LENGTH > RECEIVE_BUFFER_SIZE
#error MAX_PACKET_LENGTH cannot be greater than RECEIVE_BUFFER_SIZE
#endif

class DS2
{
public:
	DS2(DS2Bus& k, DS2Bus& l);

	/**
	 * Write a packet to the specified bus
	 *
	 * @param txPacket packet to be transmitted
	 * @param txBus which bus to transmit on
	 * @return true if packet was transmitted successfully
	 */
	bool write(const DS2Packet& txPacket, DS2BusType txBus = DS2_BOTH);

	/**
	 * Returns a packet from the specified packet buffer, or null if none.
	 *
	 * @param bus which packet buffer to read from
	 * @return pointer to new packet
	 */
	DS2Packet* read(DS2BusType bus = DS2_BOTH);

	/**
	 * Sends a packet on the specified bus and waits for a reply.
	 * Returns a new packet on the heap or null if no reply.
	 * Return pointer must be deleted if not null.
	 *
	 * @param txPacket packet to be transmitted
	 * @param txBus which bus to transmit on
	 * @param timeout_ms time to wait for reply
	 * @return reply packet
	 */
	DS2Packet* query(const DS2Packet& txPacket, DS2BusType txBus = DS2_BOTH, int timeout_ms = 600);

	/**
	 * Check for new packets in receive buffers, validate them, pop them onto
	 * the packet buffer, and call any attached callbacks.
	 * Must be run often enough to prevent receive buffer overruns.
	 */
	void task();
	
	void receiveHandlerK();
	void receiveHandlerL();

	template<typename T>
	void attach(T* classPointer, void (T::*methodPointer)(void))
	{
		if((methodPointer != 0) && (classPointer != 0))
		{
			callback.attach(classPointer, methodPointer);
		}
	}
	template<typename T>
	void detach(T* classPointer, void (T::*methodPointer)(void))
	{
		if((methodPointer != 0) && (classPointer != 0))
		{
			callback.detach(classPointer, methodPointer);
		}
	}
	void attach(void (*functionPointer)(void))
	{
		callback.attach(functionPointer);
	}
	void detach(void (*functionPointer)(void))
	{
		callback.detach(functionPointer);
	}

private:
	DS2Packet* getPacketFromBus(DS2BusType bus);
	
	DS2Bus& k;
	DS2Bus& l;
	Timer receiveTimeoutK;
	Timer receiveTimeoutL;
	std::deque<DS2Packet*> packetBufferK;
	std::deque<DS2Packet*> packetBufferL;
	FunctionPointer callback;
	volatile bool taskEnabled;
	bool interceptPackets;
	std::deque<uint8_t>* bufferK;
	std::deque<uint8_t>* bufferL;
};

#endif // DS2_H

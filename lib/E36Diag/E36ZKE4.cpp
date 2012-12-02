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

#include "E36ZKE4.h"
#include "DS2.h"

#define CMD_QUERY {0x00}
#define CMD_LOCK {0x0c, 0x5c, 0x01}
#define CMD_UNLOCK {0x0c, 0x5d, 0x01}
#define CMD_TEST {0xa0, 0x88, 0x35, 0x30, 0x95, 0x11, 0x02, 0x01, 0x00, 0x50, 0x93, 0x02, 0x11}

E36ZKE4::E36ZKE4(DS2& diagnosticInterface) : diag(diagnosticInterface)
{
	address = 0x00;
	packetType = DS2_8BIT;
}

E36ZKE4& E36ZKE4::lock()
{
	const uint8_t cmd[] = CMD_LOCK;
	DS2Packet query(address, cmd, sizeof(cmd), packetType);
	DS2Packet* reply = diag.query(query);
	if(reply != NULL)
	{
		delete reply;
	}

	return *this;
}

E36ZKE4& E36ZKE4::unlock()
{
	const uint8_t cmd[] = CMD_UNLOCK;
	DS2Packet query(address, cmd, sizeof(cmd), packetType);
	DS2Packet* reply = diag.query(query);
	if(reply != NULL)
	{
		delete reply;
	}
	
	return *this;
}

bool E36ZKE4::query()
{
	const uint8_t cmd[] = CMD_QUERY;
	DS2Packet txPacket(address, cmd, sizeof(cmd), packetType);
	DS2Packet* reply = diag.query(txPacket, DS2_BOTH);
	if(reply != NULL)
	{
		delete reply;
		return true;
	}
	return false;
}

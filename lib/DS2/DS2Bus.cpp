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

#include "DS2Bus.h"
#include "DS2.h"
#include "debugpretty.h"
#include <cstring>

DS2Bus::DS2Bus()
{
	this->bufferReadP = 0;
	this->bufferWriteP = 0;
}

int DS2Bus::read(uint8_t* buffer, int maxLength)
{
	int bytesRead = 0;
	while(bufferReadP != bufferWriteP && bytesRead < maxLength)
	{
		buffer[bytesRead++] = receiveBuffer[bufferReadP];
// 		DEBUG("reading {0x%02x} from ring buffer: 0x%x 0x%x\r\n", receiveBuffer[bufferReadP], bufferReadP, bufferWriteP);
		bufferReadP = (bufferReadP + 1) & (RECEIVE_BUFFER_SIZE - 1);
	}
	return bytesRead;
}

int DS2Bus::write(const uint8_t* data, int dataLength)
{
	if(_bus_write(data, dataLength) != dataLength)
	{
		//TODO retry write
	}
	return dataLength;
}

void DS2Bus::receiveHandler()
{
	//add the new bytes to the ring buffer
	if(!_bus_readable())
		return;

	while(_bus_readable())
	{
		if(((bufferWriteP + 1) & (RECEIVE_BUFFER_SIZE - 1)) != (bufferReadP & (RECEIVE_BUFFER_SIZE - 1)))
		{
			uint8_t c;
			_bus_read(&c, 1);
			receiveBuffer[bufferWriteP] = c;
// 			DEBUG("putting 0x%02x into DS2Bus ring buffer: 0x%x 0x%x\r\n", receiveBuffer[bufferWriteP], bufferReadP, bufferWriteP);
			bufferWriteP = (bufferWriteP + 1) & (RECEIVE_BUFFER_SIZE - 1);
		}
		else
		{
// 			DEBUG("DS2Bus ring buffer overflow\r\n");
			break;
		}
	}

	callback.call();
}

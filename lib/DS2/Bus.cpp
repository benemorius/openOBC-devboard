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

#include "Bus.h"
#include "delay.h"
#include "debugpretty.h"
#include <cstring>

Bus::Bus(Uart& uart) : uart(uart)
{
	readEnabled = true;
	uart.attach(this, &Bus::receiveHandler);
}

int Bus::_bus_read(uint8_t* buffer, int maxLength)
{
	if(!readEnabled)
		return 0;
	return uart.getdata(buffer, maxLength);
}

int Bus::_bus_write(const uint8_t* data, int dataLength)
{
	//disable reading so the echo stays here
	readEnabled = false;
	
	uart.puts(data, dataLength);
	delay(10); //FIXME more intelligent delay
	
	//receive and check echo
	uint8_t* echo = new uint8_t[dataLength];
	int length = uart.getdata(echo, dataLength);
	if(length > 0 && length < dataLength)
	{
		//wait a bit longer and check for the echo again
		delay(20);
		length = length + uart.getdata(echo + length, dataLength - length);
	}
	readEnabled = true;
	if(length == dataLength)
	{
		if(!memcmp(data, echo, dataLength))
		{
// 			DEBUG("echo ok\r\n");
		}
		else
		{
			DEBUG("echo not ok\r\n"); //TODO flush rx buffer and retry
		}
	}
	
	else
	{
		DEBUG("no echo\r\n");
		//TODO flush rx buffer and retry
	}
	delete[] echo;
	
	//if there's still more data after the echo, call the handler to receive it
	if(uart.readable())
	{
		DEBUG("still have data after echo; calling DS2Bus::receiveHandler()\r\n");
		
		uart.detach(this, &Bus::receiveHandler);
		this->DS2Bus::receiveHandler(); //incoming bytes can be missed if they arrive during here; they won't be processed until the next interrupt
		uart.attach(this, &Bus::receiveHandler);
	}
	return length;
}

bool Bus::_bus_readable()
{
	if(!readEnabled)
		return false;
	return uart.readable();
}

void Bus::receiveHandler()
{
	if(!readEnabled)
		return;
	this->DS2Bus::receiveHandler();
}

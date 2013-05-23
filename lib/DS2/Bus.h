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

#ifndef BUS_H
#define BUS_H

#include <DS2Bus.h>
#include "Uart.h"

/**
 * The Bus class extends DS2Bus to provide the hardware interface.
 * In this implementation for the current hardware, it is necessary
 * to intercept and omit the echo from the received data.
 */
class Bus : public DS2Bus
{
public:
	Bus(Uart& uart);

	virtual int _bus_read(uint8_t* buffer, int maxLength);
	virtual int _bus_write( const uint8_t* data, int dataLength);
	virtual bool _bus_readable();

	void receiveHandler();

private:
	Uart& uart;
	volatile bool readEnabled;
	 
};

#endif // BUS_H

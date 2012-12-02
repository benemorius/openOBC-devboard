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

#ifndef DS2BUS_H
#define DS2BUS_H

#define RECEIVE_BUFFER_SIZE (0x100)
#include <Timer.h>

class DS2Bus
{

public:
	DS2Bus();
	
	int read(uint8_t* buffer, int maxLength);
	int write( const uint8_t* data, int dataLength);
	bool readable() {return bufferReadP != bufferWriteP;}

	void receiveHandler();

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
	virtual int _bus_read(uint8_t* buffer, int maxLength) = 0;
	virtual int _bus_write( const uint8_t* data, int dataLength) = 0;
	virtual bool _bus_readable() = 0;
	
	volatile uint32_t bufferWriteP;
	volatile uint32_t bufferReadP;
	volatile uint8_t receiveBuffer[RECEIVE_BUFFER_SIZE];
	FunctionPointer callback;
};

#endif // DS2BUS_H

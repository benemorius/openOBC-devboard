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

#ifndef DEBUG_H
#define DEBUG_H

#include "Uart.h"
#include "Stream.h"

class Debug : public Stream
{
public:
	Debug(int8_t txPort, int8_t txPin, int8_t rxPort, int8_t rxPin, uint32_t baud, InterruptManager* interruptManager);

	void receiveHandler();

	size_t puts(const void* txbuf, uint32_t buflen);
	size_t puts(const char* string);
	size_t put(char c);
	int32_t get();
	
private:
	virtual void streamIn(const char* string);
	virtual void streamIn(uint32_t i);
	virtual void streamIn(int32_t i);
	virtual void streamIn(float f);
	virtual void streamIn(char c);
	virtual void streamIn(uint8_t c);
	
	virtual void streamOut(char* buffer);
	virtual void streamOut(char& c);
	

	Uart uart;
};

#endif // DEBUG_H

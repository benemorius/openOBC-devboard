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

#include "Debug.h"

#include "stdio.h"

#undef getc(_fp)
#undef putc(_fp)

Debug::Debug(int8_t txPort, int8_t txPin, int8_t rxPort, int8_t rxPin, uint32_t baud, InterruptManager* interruptManager) : uart(txPort, txPin, rxPort, rxPin, baud, UART_PARITY_NONE, interruptManager)
{

}

void Debug::streamIn(const char* string)
{
	uart.puts(string);
}
void Debug::streamIn(uint32_t i)
{
	char buffer[16];
	sprintf(buffer, "%i", i);
	streamIn(buffer);
}
void Debug::streamIn(int32_t i)
{
	char buffer[16];
	sprintf(buffer, "%i", i);
	streamIn(buffer);
}
void Debug::streamIn(float f)
{
	char buffer[16];
	sprintf(buffer, "%0.1f", f);
	streamIn(buffer);
}
void Debug::streamIn(char c)
{
	uart.putc(c);
}
void Debug::streamIn(uint8_t c)
{
	uart.putc(c);
}
void Debug::streamOut(char* buffer)
{
	*buffer = (char)get();
}
void Debug::streamOut(char& c)
{
	c = (char)get();
}

void Debug::receiveHandler()
{
	uart.receiveHandler();
}

size_t Debug::put(char c)
{
	uart.putc(c);
}

size_t Debug::puts(const void* txbuf, uint32_t buflen)
{
	uart.puts(txbuf, buflen);
}

size_t Debug::puts(const char* string)
{
	uart.puts(string);
}

int32_t Debug::get()
{
	return uart.getc();
}

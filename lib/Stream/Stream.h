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

#ifndef STREAM_H
#define STREAM_H
#include <stdint.h>

class Stream
{
	friend Stream& operator<<(Stream& stream, const char* string)
	{
		stream.streamIn(string);
		return stream;
	}
	friend Stream& operator<<(Stream& stream, uint32_t i)
	{
		stream.streamIn(i);
		return stream;
	}
	friend Stream& operator<<(Stream& stream, int32_t i)
	{
		stream.streamIn(i);
		return stream;
	}
	friend Stream& operator<<(Stream& stream, float f)
	{
		stream.streamIn(f);
		return stream;
	}
	friend Stream& operator<<(Stream& stream, char c)
	{
		stream.streamIn(c);
		return stream;
	}
	friend Stream& operator<<(Stream& stream, uint8_t c)
	{
		stream.streamIn(c);
		return stream;
	}
	
	friend Stream& operator>>(Stream& stream, char* buffer)
	{
		stream.streamOut(buffer);
		return stream;
	}
	friend Stream& operator>>(Stream& stream, char& c)
	{
		stream.streamOut(c);
		return stream;
	}

public:
	virtual void seek(uint32_t offset) {};
	virtual uint32_t write(const void* data, uint32_t length) {};
	virtual uint32_t read(void* buffer, uint32_t maxlength) {};
	virtual uint32_t gets(void* buffer, uint32_t maxlength) {};

private:
	virtual void streamIn(const char* string) = 0;
	virtual void streamIn(uint32_t i) = 0;
	virtual void streamIn(int32_t i) = 0;
	virtual void streamIn(float f) = 0;
	virtual void streamIn(char c) = 0;
	virtual void streamIn(uint8_t c) = 0;
	
	virtual void streamOut(char* buffer) = 0;
	virtual void streamOut(char& c) = 0;
	
	
};

#endif // STREAM_H

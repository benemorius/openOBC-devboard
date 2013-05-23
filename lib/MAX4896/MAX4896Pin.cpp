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



#include "MAX4896Pin.h"
#include "MAX4896.h"

MAX4896Pin::MAX4896Pin(MAX4896& max, uint8_t bitmask, bool isOn, bool onIsHigh) : IO(0xff, 0, isOn, onIsHigh), max(max), bitmask(bitmask)
{
	setState(isOn);
}

void MAX4896Pin::setState(bool state)
{
	this->state = state;
	uint8_t bits = max.readBits();
	bits &= ~bitmask;
	if(state ^ onIsHigh)
		bits |= bitmask;
	max.writeBits(bits);
}

bool MAX4896Pin::getState() const
{
	return this->state;
}

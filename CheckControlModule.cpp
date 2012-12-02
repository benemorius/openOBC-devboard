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

#include "CheckControlModule.h"
#include "delay.h"

CheckControlModule::CheckControlModule(Input& data, IO& clock, IO& latch) : data(data), clock(clock), latch(latch)
{
	clock = false;
	latch = false;
	updateStatus();
}

void CheckControlModule::task()
{
	updateStatus();
}

void CheckControlModule::updateStatus()
{
	uint8_t status = 0;
	clock = true;
	latch = true;
	for(uint8_t bit = 8; bit; bit--)
	{
		delay(1); //TODO shorten these delays
		clock = false;
		delay(1);
		clock = true;
		delay(1);
		if(data)
			status += (1<<(bit-1));
	}
	latch = false;
	clock = false;
	rawByte = status;
}

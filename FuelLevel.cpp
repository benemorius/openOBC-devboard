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

#include "FuelLevel.h"
#include "delay.h"
#include "Timer.h"

FuelLevel::FuelLevel(Input& dataPin) : dataPin(dataPin)
{
	dataPin.setTriState();
}

uint32_t FuelLevel::getLitres()
{
	uint32_t litres = 0;

	Timer timeout;
	while(dataPin && timeout.read_ms() < 1000); //wait for start bit - up to 1000ms
	delay(2); //wait for middle of bit
	delay(4); //wait for next bit

	for(uint8_t bit = 0; bit < 10; ++bit)
	{
		if(dataPin)
			litres += (1<<bit);
		delay(4);
	}
	bool zeroBit = dataPin; //TODO verify always zero
	delay(4);
	bool parityBit = dataPin; //TODO verify parity

	return litres;
}

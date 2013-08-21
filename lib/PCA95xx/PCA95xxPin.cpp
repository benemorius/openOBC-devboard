/*
    Copyright (c) 2013 <benemorius@gmail.com>

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


#include "PCA95xxPin.h"

PCA95xxPin::PCA95xxPin(PCA95xx& pca, uint8_t port, uint8_t pin, bool isOutput, bool isOn, bool onIsHigh) : IO(0xff, 0, isOutput, onIsHigh), pca(pca), port(port), pin(pin), isOutput(isOutput)
{
	bitmask = (1 << pin) << (port * 8);
	
	setState(isOn);
	if(isOutput)
		setOutput();
}

PCA95xxPin::~PCA95xxPin()
{
	setInput();
	setState(false);
}

void PCA95xxPin::setState(bool state)
{
	this->state = state;
	uint16_t bits = pca.getCurrentOutputBits();
	bits &= ~bitmask;
	if(state ^ !onIsHigh)
		bits |= bitmask;
	pca.writeBits(bits);
}

bool PCA95xxPin::getState() const
{
	if(isOutput)
		return state;
	uint16_t bits = pca.readBits();
	return (bits >> (port * 8)) & (1<<pin);
}

void PCA95xxPin::setInput()
{
	pca.setInput(port, pin);
}

void PCA95xxPin::setOutput()
{
	pca.setOutput(port, pin);
}

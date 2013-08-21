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


#include "PCA95xx.h"

PCA95xx::PCA95xx(I2C& i2c, uint8_t address, Input& interrupt, uint32_t hz) : interruptManager(interruptManager), i2c(i2c), address(address), interrupt(interrupt), hz(hz)
{
	regO0 = 0x00;
	regO1 = 0x00;
	regC0 = 0xff;
	regC1 = 0xff;
	regN0 = 0x00;
	regN1 = 0x00;
	writeRegister(PCA95XX_REG_C0, regC0);
	writeRegister(PCA95XX_REG_C1, regC1);
	writeRegister(PCA95XX_REG_N0, regN0);
	writeRegister(PCA95XX_REG_N1, regN1);
	outputBits = 0x0000;
	writeBits(outputBits);
}

PCA95xx::~PCA95xx()
{
	writeRegister(PCA95XX_REG_C0, 0xff);
	writeRegister(PCA95XX_REG_C1, 0xff);
	writeRegister(PCA95XX_REG_N0, 0x00);
	writeRegister(PCA95XX_REG_N1, 0x00);
	writeBits(0x0000);
}

uint16_t PCA95xx::readRegister(uint8_t reg)
{
	i2c.setFrequency(hz);
	uint8_t writeBuf[1];
	uint8_t readBuf[2];
	writeBuf[0] = reg;
	if(i2c.readwrite(address, writeBuf, 1, readBuf, 2) == sizeof(readBuf))
	{
		uint16_t bits = readBuf[0];
		bits += (readBuf[1] << 8);
		return bits;
	}
	return 0xffff;
}

void PCA95xx::writeRegister(uint8_t reg, uint8_t data)
{
	i2c.setFrequency(hz);
	uint8_t buf[2];
	buf[0] = reg;
	buf[1] = data;
	i2c.write(address, buf, 2);
}

uint16_t PCA95xx::readBits()
{
	return readRegister(PCA95XX_REG_I0);
}

PCA95xx& PCA95xx::writeBits(uint16_t outputBits)
{
	this->outputBits = outputBits;
	i2c.setFrequency(hz);

	uint8_t buf[3];
	buf[0] = PCA95XX_REG_O0;
	buf[1] = outputBits & 0xff;
	buf[2] = (outputBits >> 8) & 0xff;
	i2c.write(address, buf, 3);
	return *this;
}

PCA95xx& PCA95xx::setFrequency(uint32_t hz)
{
	this->hz = hz;
	return *this;
}

void PCA95xx::setInput(uint8_t port, uint8_t pin)
{
	if(port == 0)
	{
		regC0 |= (1<<pin);
		writeRegister(PCA95XX_REG_C0, regC0);
	}		
	else if(port == 1)
	{
		regC1 |= (1<<pin);
		writeRegister(PCA95XX_REG_C1, regC1);
	}
}

void PCA95xx::setOutput(uint8_t port, uint8_t pin)
{
	if(port == 0)
	{
		regC0 &= ~(1<<pin);
		writeRegister(PCA95XX_REG_C0, regC0);
	}		
	else if(port == 1)
	{
		regC1 &= ~(1<<pin);
		writeRegister(PCA95XX_REG_C1, regC1);
	}
}

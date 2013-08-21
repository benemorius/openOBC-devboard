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


#include "MMA845x.h"

MMA845x::MMA845x(I2C& i2c, uint8_t address, Input& interrupt, InterruptManager& interruptManager, uint32_t hz) : interruptManager(interruptManager), i2c(i2c), address(address), hz(hz), interrupt(interrupt)
{
	enable();
}

MMA845x::~MMA845x()
{
	disable();
}

uint8_t MMA845x::readRegister(uint8_t reg)
{
	i2c.setFrequency(hz);
	uint8_t writeBuf;
	uint8_t readBuf;
	writeBuf = reg;
	if(i2c.readwrite(address, &writeBuf, 1, &readBuf, 1) == 1)
	{
		return readBuf;
	}
	return 0xff;
}

void MMA845x::writeRegister(uint8_t reg, uint8_t data)
{
	i2c.setFrequency(hz);
	uint8_t buf[2];
	buf[0] = reg;
	buf[1] = data;
	i2c.write(address, buf, 2);
}

MMA845x& MMA845x::setFrequency(uint32_t hz)
{
	this->hz = hz;
	return *this;
}

MMA845x& MMA845x::enable()
{
	writeRegister(MMA845X_REG_CTRL_REG1, 0x01); //enable sampling
	
}

MMA845x& MMA845x::disable()
{
	writeRegister(MMA845X_REG_CTRL_REG1, 0x00); //disable sampling
}

float MMA845x::getX()
{
	int16_t x = ((int16_t)readRegister(MMA845X_REG_OUT_X_MSB) << 8);
	x /= (1<<4);
	x += (uint16_t)readRegister(MMA845X_REG_OUT_X_LSB) >> 4;
	return (float)x / 2048 * 2;
}

float MMA845x::getY()
{
	int16_t y = ((int16_t)readRegister(MMA845X_REG_OUT_Y_MSB) << 8);
	y /= (1<<4);
	y += (uint16_t)readRegister(MMA845X_REG_OUT_Y_LSB) >> 4;
	return (float)y / 2048 * 2;
}

float MMA845x::getZ()
{
	int16_t z = ((int16_t)readRegister(MMA845X_REG_OUT_Z_MSB) << 8);
	z /= (1<<4);
	z += (int16_t)readRegister(MMA845X_REG_OUT_Z_LSB) >> 4;
	return (float)z / 2048 * 2;
}

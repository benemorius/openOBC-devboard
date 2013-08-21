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

#ifndef MMA845X_H
#define MMA845X_H

#include <I2C.h>
#include <Input.h>
#include <InterruptManager.h>

#define MMA845X_REG_CTRL_REG1 (0x2a)
#define MMA845X_REG_CTRL_REG2 (0x2b)
#define MMA845X_REG_CTRL_REG3 (0x2c)
#define MMA845X_REG_CTRL_REG4 (0x2d)
#define MMA845X_REG_CTRL_REG5 (0x2e)
#define MMA845X_REG_OUT_X_MSB (0x01)
#define MMA845X_REG_OUT_X_LSB (0x02)
#define MMA845X_REG_OUT_Y_MSB (0x03)
#define MMA845X_REG_OUT_Y_LSB (0x04)
#define MMA845X_REG_OUT_Z_MSB (0x05)
#define MMA845X_REG_OUT_Z_LSB (0x06)
#define MMA845X_REG_XYZ_DATA_CFG (0x0e)

class MMA845x
{

public:
	MMA845x(I2C& i2c, uint8_t address, Input& interrupt, InterruptManager& interruptManager, uint32_t hz = 400000);
	~MMA845x();
	
	MMA845x& setFrequency(uint32_t hz);
	float getX();
	float getY();
	float getZ();
	
	MMA845x& enable();
	MMA845x& disable();
	
private:
	uint8_t readRegister(uint8_t reg);
	void writeRegister(uint8_t reg, uint8_t data);
	
	I2C& i2c;
	uint8_t address;
	uint32_t hz;
	Input& interrupt;
	InterruptManager& interruptManager;
};

#endif // MMA845X_H

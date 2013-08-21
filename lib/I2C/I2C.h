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


#ifndef I2C_H
#define I2C_H

#include <stdint.h>
#include <lpc17xx_i2c.h>

class I2C
{

public:
	I2C(uint8_t sdaPort, uint8_t sdaPin, uint8_t sclPort, uint8_t sclPin, uint32_t hz = 100000);
	~I2C();
	
	I2C& setFrequency(uint32_t hz);
	uint32_t read(uint8_t address, uint8_t* buffer, uint32_t length);
	uint32_t write(uint8_t address, uint8_t* data, uint32_t length);
	uint32_t readwrite(uint8_t address, uint8_t* writeData, uint32_t writeLength, uint8_t* readBuffer, uint32_t readLength);
	
private:
	uint8_t sdaPort;
	uint8_t sdaPin;
	uint8_t sclPort;
	uint8_t sclPin;
	uint32_t hz;
	LPC_I2C_TypeDef* peripheral;
};

#endif // I2C_H

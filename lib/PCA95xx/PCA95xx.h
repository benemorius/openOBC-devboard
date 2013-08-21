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

#ifndef PCA95XX_H
#define PCA95XX_H

#include <I2C.h>
#include <Input.h>
#include <InterruptManager.h>
#include <stdint.h>


#define PCA95XX_REG_I0 (0)
#define PCA95XX_REG_I1 (1)
#define PCA95XX_REG_O0 (2)
#define PCA95XX_REG_O1 (3)
#define PCA95XX_REG_N0 (4)
#define PCA95XX_REG_N1 (5)
#define PCA95XX_REG_C0 (6)
#define PCA95XX_REG_C1 (7)

class PCA95xx
{
public:
	PCA95xx(I2C& i2c, uint8_t address, Input& interrupt, uint32_t hz = 400000);
	~PCA95xx();
	
	uint16_t readBits();
	PCA95xx& writeBits(uint16_t outputBits);
	uint16_t getCurrentOutputBits() {return outputBits;}
	
	void setInput(uint8_t port, uint8_t pin);
	void setOutput(uint8_t port, uint8_t pin);
	
	PCA95xx& setFrequency(uint32_t hz);
	
	
private:
	uint16_t readRegister(uint8_t reg);
	void writeRegister(uint8_t reg, uint8_t data);
	
	I2C& i2c;
	uint8_t address;
	Input& interrupt;
	uint16_t outputBits;
	uint32_t hz;
	uint16_t currentOutputBits;
	InterruptManager& interruptManager;
	uint8_t regO0;
	uint8_t regO1;
	uint8_t regN0;
	uint8_t regN1;
	uint8_t regC0;
	uint8_t regC1;
};

#endif // PCA95XX_H

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

#ifndef ANALOGIN_H
#define ANALOGIN_H

#include <lpc17xx_adc.h>

class AnalogIn
{
public:
	AnalogIn(uint8_t port, uint8_t pin, float referenceVoltage = 3.0f, float scaleVoltage = 0.0f);

	float read();
	float readPercent();
	uint16_t readBits();

	uint8_t getPort() const {return port;}
	uint8_t getPin() const {return pin;}
	float getReferenceVoltage() const {return referenceVoltage;}
	float getScaleVoltage() const {return scaleVoltage;}

private:
	uint8_t port;
	uint8_t pin;
	float referenceVoltage;
	float scaleVoltage;
	ADC_CHANNEL_SELECTION channel;
	ADC_TYPE_INT_OPT interrupt;
};

#endif // ANALOGIN_H

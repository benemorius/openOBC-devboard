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


#include "AnalogOut.h"
#include <lpc17xx_pinsel.h>
#include <lpc17xx_dac.h>

#define MAX_RAW_VALUE (0x3ff)

AnalogOut::AnalogOut(uint8_t port, uint8_t pin, float maxScaleVoltage) : port(port), pin(pin), maxScaleVoltage(maxScaleVoltage)
{
	PINSEL_CFG_Type PinCfg;
	if(port == 0 && pin == 26)
	{
		PinCfg.Funcnum = PINSEL_FUNC_2;
		peripheral = LPC_DAC;
	}
	else
	{
		//invalid port/pin specified
		while(1);
	}
	
	PinCfg.OpenDrain = PINSEL_PINMODE_NORMAL;
	PinCfg.Pinmode = PINSEL_PINMODE_TRISTATE;
	PinCfg.Portnum = port;
	PinCfg.Pinnum = pin;
	PINSEL_ConfigPin(&PinCfg);
	
	DAC_Init(peripheral);
	currentRawValue = 0;
	writeRaw(0x000);
}

AnalogOut::~AnalogOut()
{
// 	DAC_DeInit();
}

uint16_t AnalogOut::readRaw()
{
	return currentRawValue;
}
void AnalogOut::writeRaw(uint16_t rawValue)
{
	DAC_UpdateValue(peripheral, rawValue);
}

float AnalogOut::readVoltage()
{
	return (float)readRaw() * maxScaleVoltage;
}

float AnalogOut::readPercent()
{
	return (float)currentRawValue / MAX_RAW_VALUE;
}

void AnalogOut::writeVoltage(float voltage)
{
	writePercent(voltage / maxScaleVoltage);
}

void AnalogOut::writePercent(float percent)
{
	uint16_t rawValue = percent * MAX_RAW_VALUE;
	writeRaw(rawValue);
}

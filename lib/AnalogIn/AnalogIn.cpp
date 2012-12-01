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

#include "AnalogIn.h"
#include <lpc17xx_pinsel.h>
#include <cstdio>

AnalogIn::AnalogIn(uint8_t port, uint8_t pin, float referenceVoltage, float scaleVoltage) : port(port), pin(pin), referenceVoltage(referenceVoltage), scaleVoltage(scaleVoltage)
{
	if(this->scaleVoltage == 0.0f)
		this->scaleVoltage = referenceVoltage;
	
	uint8_t funcnum;
	if(port == 0 && pin == 23)
	{
		funcnum = 1;
		channel = ADC_CHANNEL_0;
		interrupt = ADC_ADINTEN0;
	}
	else if(port == 0 && pin == 24)
	{
		funcnum = 1;
		channel = ADC_CHANNEL_1;
		interrupt = ADC_ADINTEN1;
	}
	else if(port == 0 && pin == 25)
	{
		funcnum = 1;
		channel = ADC_CHANNEL_2;
		interrupt = ADC_ADINTEN2;
	}
	else if(port == 0 && pin == 26)
	{
		funcnum = 1;
		channel = ADC_CHANNEL_3;
		interrupt = ADC_ADINTEN3;
	}
	else if(port == 1 && pin == 30)
	{
		funcnum = 1;
		channel = ADC_CHANNEL_4;
		interrupt = ADC_ADINTEN4;
	}
	else if(port == 1 && pin == 31)
	{
		funcnum = 1;
		channel = ADC_CHANNEL_5;
		interrupt = ADC_ADINTEN5;
	}
	else
	{
		//invalid port/pin
		fprintf(stderr, "invalid pin specified\r\n");
		throw;
	}
	
	PINSEL_CFG_Type PinCfg;
	PinCfg.Funcnum = funcnum;
	PinCfg.OpenDrain = PINSEL_PINMODE_NORMAL;
	PinCfg.Pinmode = PINSEL_PINMODE_TRISTATE;
	PinCfg.Portnum = port;
	PinCfg.Pinnum = pin;
	PINSEL_ConfigPin(&PinCfg);

	ADC_Init(LPC_ADC, 200000);
	ADC_IntConfig(LPC_ADC,interrupt,DISABLE);
}

float AnalogIn::read()
{
	return readPercent() * scaleVoltage;
}

float AnalogIn::readPercent()
{
	return (float)readBits() / 0xfff;
}

uint16_t AnalogIn::readBits()
{
	ADC_ChannelCmd(LPC_ADC, channel, ENABLE);
	ADC_StartCmd(LPC_ADC, ADC_START_NOW);
	while(!(ADC_ChannelGetStatus(LPC_ADC, channel, ADC_DATA_DONE)));
	uint16_t bits = ADC_ChannelGetData(LPC_ADC, channel);
	ADC_ChannelCmd(LPC_ADC, channel, DISABLE);
	return bits;
}

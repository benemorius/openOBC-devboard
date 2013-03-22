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

#include "IO.h"

#include <lpc17xx_gpio.h>
#include <lpc17xx_pinsel.h>
#include <stdio.h>

IO::IO(uint8_t port, uint8_t pin, bool isOn, bool onIsHigh)
{
	this->port = port;
	this->pin = pin;
	this->onIsHigh = onIsHigh;
	this->isOn = isOn;

	PINSEL_CFG_Type PinCfg;
	PinCfg.Funcnum = PINSEL_FUNC_0;
	PinCfg.OpenDrain = PINSEL_PINMODE_NORMAL;
	PinCfg.Pinmode = PINSEL_PINMODE_TRISTATE;
	PinCfg.Portnum = this->port;
	PinCfg.Pinnum = this->pin;
	PINSEL_ConfigPin(&PinCfg);

	setState(this->isOn);
	
	GPIO_SetDir(port, (1<<pin), 1);
}

void IO::setOpenDrain(bool isOpenDrain)
{
	this->isOpenDrain = isOpenDrain;
	
	PINSEL_CFG_Type PinCfg;
	PinCfg.Funcnum = PINSEL_FUNC_0;
	if(isOpenDrain)
		PinCfg.OpenDrain = PINSEL_PINMODE_OPENDRAIN;
	else
		PinCfg.OpenDrain = PINSEL_PINMODE_NORMAL;
	PinCfg.Pinmode = PINSEL_PINMODE_TRISTATE;
	PinCfg.Portnum = this->port;
	PinCfg.Pinnum = this->pin;
	PINSEL_ConfigPin(&PinCfg);
}

void IO::on()
{
	setState(true);
}

void IO::off()
{
	setState(false);
}

void IO::toggle()
{
	if(isOn)
		off();
	else
		on();
	
}

bool IO::getState() const
{
	if(this->onIsHigh)
		return (GPIO_ReadValue(this->port) & (1<<this->pin));
	else
		return !(GPIO_ReadValue(this->port) & (1<<this->pin));
}

void IO::setState(bool on)
{
	isOn = on;
	
	if((isOn && onIsHigh) || (!isOn && !onIsHigh))
	{
		GPIO_SetValue(port, (1<<pin));
	}
	else
	{
		GPIO_ClearValue(port, (1<<pin));
	}
}

IO& IO::operator=(bool state)
{
	setState(state);
	return *this;
}

void IO::setInput()
{
	GPIO_SetDir(port, (1<<pin), 0);
}

void IO::setOutput()
{
	GPIO_SetDir(port, (1<<pin), 1);
}

void IO::setPullup()
{
	PINSEL_CFG_Type PinCfg;
	PinCfg.Funcnum = PINSEL_FUNC_0;
	if(isOpenDrain)
		PinCfg.OpenDrain = PINSEL_PINMODE_OPENDRAIN;
	else
		PinCfg.OpenDrain = PINSEL_PINMODE_NORMAL;
	PinCfg.Pinmode = PINSEL_PINMODE_PULLUP;
	PinCfg.Portnum = this->port;
	PinCfg.Pinnum = this->pin;
	PINSEL_ConfigPin(&PinCfg);
}

void IO::setPulldown()
{
	PINSEL_CFG_Type PinCfg;
	PinCfg.Funcnum = PINSEL_FUNC_0;
	if(isOpenDrain)
		PinCfg.OpenDrain = PINSEL_PINMODE_OPENDRAIN;
	else
		PinCfg.OpenDrain = PINSEL_PINMODE_NORMAL;
	PinCfg.Pinmode = PINSEL_PINMODE_PULLDOWN;
	PinCfg.Portnum = this->port;
	PinCfg.Pinnum = this->pin;
	PINSEL_ConfigPin(&PinCfg);
}

void IO::setTristate()
{
	PINSEL_CFG_Type PinCfg;
	PinCfg.Funcnum = PINSEL_FUNC_0;
	if(isOpenDrain)
		PinCfg.OpenDrain = PINSEL_PINMODE_OPENDRAIN;
	else
		PinCfg.OpenDrain = PINSEL_PINMODE_NORMAL;
	PinCfg.Pinmode = PINSEL_PINMODE_TRISTATE;
	PinCfg.Portnum = this->port;
	PinCfg.Pinnum = this->pin;
	PINSEL_ConfigPin(&PinCfg);
}

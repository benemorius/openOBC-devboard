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

#include "Input.h"
#include <lpc17xx_pinsel.h>
#include <lpc17xx_gpio.h>

Input::Input(uint8_t port, uint8_t pin, bool onIsHigh) : port(port), pin(pin), onIsHigh(onIsHigh)
{
	PINSEL_CFG_Type PinCfg;
	PinCfg.Funcnum = PINSEL_FUNC_0;
	PinCfg.OpenDrain = PINSEL_PINMODE_NORMAL;
	PinCfg.Pinmode = PINSEL_PINMODE_TRISTATE;
	PinCfg.Portnum = this->port;
	PinCfg.Pinnum = this->pin;
	PINSEL_ConfigPin(&PinCfg);
	
	GPIO_SetDir(port, (1<<pin), 0);
}

bool Input::getState() const
{
	if(this->onIsHigh)
		return (GPIO_ReadValue(this->port) & (1<<this->pin));
	else
		return !(GPIO_ReadValue(this->port) & (1<<this->pin));
}

void Input::setPullup()
{
	PINSEL_CFG_Type PinCfg;
	PinCfg.Funcnum = PINSEL_FUNC_0;
	PinCfg.OpenDrain = PINSEL_PINMODE_NORMAL;
	PinCfg.Pinmode = PINSEL_PINMODE_PULLUP;
	PinCfg.Portnum = this->port;
	PinCfg.Pinnum = this->pin;
	PINSEL_ConfigPin(&PinCfg);
}

void Input::setPulldown()
{
	PINSEL_CFG_Type PinCfg;
	PinCfg.Funcnum = PINSEL_FUNC_0;
	PinCfg.OpenDrain = PINSEL_PINMODE_NORMAL;
	PinCfg.Pinmode = PINSEL_PINMODE_PULLDOWN;
	PinCfg.Portnum = this->port;
	PinCfg.Pinnum = this->pin;
	PINSEL_ConfigPin(&PinCfg);
}

void Input::setTristate()
{
	PINSEL_CFG_Type PinCfg;
	PinCfg.Funcnum = PINSEL_FUNC_0;
	PinCfg.OpenDrain = PINSEL_PINMODE_NORMAL;
	PinCfg.Pinmode = PINSEL_PINMODE_TRISTATE;
	PinCfg.Portnum = this->port;
	PinCfg.Pinnum = this->pin;
	PINSEL_ConfigPin(&PinCfg);
}

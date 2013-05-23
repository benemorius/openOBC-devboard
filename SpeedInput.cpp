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

#include "SpeedInput.h"
#include <lpc17xx_gpio.h>

#define MAX_PERIOD (2000000) //2000000us == ~0.24mph

SpeedInput::SpeedInput(Input& input, InterruptManager& interruptManager) : input(input), interruptManager(interruptManager)
{
	interruptManager.attach(IRQ_EINT3, this, &SpeedInput::interruptHandler);
	GPIO_IntEnable(input.getPort(), (1<<input.getPin()), 1);
	NVIC_EnableIRQ(EINT3_IRQn);

}

SpeedInput::~SpeedInput()
{
	GPIO_IntDisable(input.getPort(), (1<<input.getPin()), 1);
	interruptManager.detach(IRQ_EINT3, this, &SpeedInput::interruptHandler);
}

float SpeedInput::getSpeed()
{
	uint32_t currentPeriod = periodTimer.read_us();
	if(currentPeriod > MAX_PERIOD)
	{
		currentSpeed = 0.0f;
		return currentSpeed;
	}
	else
	{
		currentSpeed = (float)1000000 / lastPeriod_us / 4712 * 60 * 60;
		return currentSpeed;
	}
}

void SpeedInput::interruptHandler()
{
	if(GPIO_GetIntStatus(input.getPort(), input.getPin(), 1))
	{
		uint32_t currentPeriod = periodTimer.read_us();
		periodTimer.start();
		GPIO_ClearInt(input.getPort(), (1<<input.getPin()));
		lastPeriod_us = currentPeriod;
	}
}

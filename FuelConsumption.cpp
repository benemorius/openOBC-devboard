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

#include "FuelConsumption.h"
#include <lpc17xx_gpio.h>

#define MAX_PERIOD (500000) //us

FuelConsumption::FuelConsumption(Input& input, InterruptManager& interruptManager) : input(input), interruptManager(interruptManager)
{
	period_us = 0;
	ontime_us = 0;
	interruptManager.attach(IRQ_EINT3, this, &FuelConsumption::interruptHandler);
	GPIO_IntEnable(input.getPort(), (1<<input.getPin()), 0);
	GPIO_IntEnable(input.getPort(), (1<<input.getPin()), 1);
	NVIC_EnableIRQ(EINT3_IRQn);
}

FuelConsumption::~FuelConsumption()
{
	GPIO_IntDisable(input.getPort(), (1<<input.getPin()), 0);
	GPIO_IntDisable(input.getPort(), (1<<input.getPin()), 1);
	interruptManager.detach(IRQ_EINT3, this, &FuelConsumption::interruptHandler);
}

float FuelConsumption::getRpm()
{
	uint32_t period = getPeriod_us();
	if(period == 0)
		return 0;
	return (float)1 / ((float)period / 1000000) * 60;
}

float FuelConsumption::getDutyCycle()
{
	uint32_t period = getPeriod_us();
	uint32_t ontime = getOntime_us();
	if(period == 0 || ontime == 0)
		return 0.0f;
	return (float)ontime / (float)period;
}

uint32_t FuelConsumption::getPeriod_us()
{
	if(timeSinceLastFallingEdge.read_us() > MAX_PERIOD)
		return 0;
	return period_us;
}

uint32_t FuelConsumption::getOntime_us()
{
	if(timeSinceLastFallingEdge.read_us() > MAX_PERIOD)
		return 0;
	return ontime_us;
}

void FuelConsumption::interruptHandler()
{
	bool edgeIsRising;
	if(GPIO_GetIntStatus(input.getPort(), input.getPin(), 1)) //falling edge
		edgeIsRising = false;
	else if(GPIO_GetIntStatus(input.getPort(), input.getPin(), 0)) //rising edge
		edgeIsRising = true;
	else
		return;
	GPIO_ClearInt(input.getPort(), (1<<input.getPin()));

	if(!edgeIsRising)
	{
		this->period_us = timeSinceLastFallingEdge.read_us();
		timeSinceLastFallingEdge.start();
		return;
	}

	if(edgeIsRising)
	{
		this->ontime_us = timeSinceLastFallingEdge.read_us();
	}
}

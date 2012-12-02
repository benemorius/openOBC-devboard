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

#include "FuelLevel.h"
#include "Timer.h"
#include <lpc17xx_gpio.h>

#define TIMEOUT (10000) //us
#define BIT_PERIOD (4000) //us

FuelLevel::FuelLevel(Input& dataPin, InterruptManager& interruptManager) : dataPin(dataPin), interruptManager(interruptManager)
{
	decilitres = 0;
	interruptManager.attach(IRQ_EINT3, this, &FuelLevel::interruptHandler);
	GPIO_IntEnable(dataPin.getPort(), (1<<dataPin.getPin()), 0);
	GPIO_IntEnable(dataPin.getPort(), (1<<dataPin.getPin()), 1);
	NVIC_EnableIRQ(EINT3_IRQn);
}

FuelLevel::~FuelLevel()
{
	GPIO_IntDisable(dataPin.getPort(), (1<<dataPin.getPin()), 0);
	GPIO_IntDisable(dataPin.getPort(), (1<<dataPin.getPin()), 1);
	interruptManager.detach(IRQ_EINT3, this, &FuelLevel::interruptHandler);
}

void FuelLevel::interruptHandler()
{
	static bool haveStartBit;
	static bool currentLogicState;
	static uint8_t currentBit;
	static uint16_t rxData;
	
	bool edgeIsHigh;
	if(GPIO_GetIntStatus(dataPin.getPort(), dataPin.getPin(), 1)) //falling edge
		edgeIsHigh = false;
	else if(GPIO_GetIntStatus(dataPin.getPort(), dataPin.getPin(), 0)) //rising edge
		edgeIsHigh = true;
	else
		return;

	GPIO_ClearInt(dataPin.getPort(), (1<<dataPin.getPin()));

	uint32_t currentPeriod = periodTimer.read_us();
	periodTimer.start();

	if(currentPeriod >= BIT_PERIOD * 15) //packet timeout
		haveStartBit = false;
	
	if(!haveStartBit) //start new packet
	{
		haveStartBit = true;
		currentLogicState = false;
		currentBit = 0;
		rxData = 0;
		return;
	}
	//else continue existing packet

	//count up the number of bit periods since the last edge and set the bits
	if(currentBit > 0)
		currentBit = currentBit;
	uint8_t state = currentLogicState ? 1 : 0;
	while(currentPeriod >= BIT_PERIOD/2)
	{
		rxData += (state<<currentBit++);

		if(currentPeriod < BIT_PERIOD)
			break;
		currentPeriod -= BIT_PERIOD;
	}

	//if we got an edge here then the parity bit is high and there won't be any more edges
	if(currentBit == 12)
	{
		state = edgeIsHigh ? 1 : 0;
		rxData += state << currentBit++;
	}

	if(currentBit == 13) //just received the last bit
	{
		bool parityBit = rxData & 0x1000;
		//TODO verify parity

		rxData &= ~0x1800; //clear parity bit and zero bit
		rxData >>= 1; //shift off the start bit
		this->decilitres = rxData;
	}
	if(currentBit >= 13) //end of packet; set up to start a new packet at the next edge
	{
		haveStartBit = false;
	}

	currentLogicState = edgeIsHigh;
}

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

#ifndef CAPTURE_H
#define CAPTURE_H

#include "InterruptManager.h"
#include "FunctionPointer.h"

#include <LPC17xx.h>
#include <stdint.h>
#include <lpc17xx_clkpwr.h>

class Capture
{
public:
	Capture(uint8_t speedPort, uint8_t speedPin, uint8_t fuelConsPort, uint8_t fuelConsPin, InterruptManager* interruptManager);

	void interruptHandler();
	float getPeriod() {return period;}
	float getFrequency() {return (float)1 / period;}
	float getDutyCycle() {return dutyCycle;}

	float getOnTime() {return onTime;}

	template<typename T>
	void attach(T* classPointer, void (T::*methodPointer)(void))
	{
		if((methodPointer != 0) && (classPointer != 0))
		{
			callback.attach(classPointer, methodPointer);
		}
	}
	void attach(void (*functionPointer)(void))
	{
		callback.attach(functionPointer);
	}

private:
	uint8_t speedPort;
	uint8_t speedPin;
	uint8_t fuelConsPort;
	uint8_t fuelConsPin;
	LPC_TIM_TypeDef* peripheral;
	uint8_t channel;
	IRQn_Type irqn;
	FunctionPointer callback;
	uint32_t highCount;
	uint32_t risingEdge;
	uint32_t fallingEdge;
	uint32_t lastFallingEdge;
	bool high;
	float frequency;
	float dutyCycle;
	float period;
	float onTime;
	
	
};

#endif // CAPTURE_H

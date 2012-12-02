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

#include "ObcKeypad.h"
#include <lpc17xx_gpio.h>

ObcKeypad::ObcKeypad(IO& x0, IO& x1, IO& x2, IO& x3, IO& x4, Input& y0, Input& y1, Input& y2, Input& y3, InterruptManager& interruptManager) : x0(x0), x1(x1), x2(x2), x3(x3), x4(x4), y0(y0), y1(y1), y2(y2), y3(y3), interruptManager(interruptManager)
{
	x0.setOpenDrain(true);
	x1.setOpenDrain(true);
	x2.setOpenDrain(true);
	x3.setOpenDrain(true);
	x4.setOpenDrain(true);
	x0 = false;
	x1 = false;
	x2 = false;
	x3 = false;
	x4 = false;
	y0.setPullup();
	y1.setPullup();
	y2.setPullup();
	y3.setPullup();

	for(uint32_t i = 0; i < BUTTON_NUM_VALUES; i++)
		callbacks[i] = 0;

	interruptManager.attach(IRQ_EINT3, this, &ObcKeypad::interruptHandler);
	GPIO_IntEnable(y0.getPort(), (1<<y0.getPin()), 1);
	GPIO_IntEnable(y1.getPort(), (1<<y1.getPin()), 1);
	GPIO_IntEnable(y2.getPort(), (1<<y2.getPin()), 1);
	GPIO_IntEnable(y3.getPort(), (1<<y3.getPin()), 1);
	GPIO_IntEnable(y0.getPort(), (1<<y0.getPin()), 0);
	GPIO_IntEnable(y1.getPort(), (1<<y1.getPin()), 0);
	GPIO_IntEnable(y2.getPort(), (1<<y2.getPin()), 0);
	GPIO_IntEnable(y3.getPort(), (1<<y3.getPin()), 0);
	NVIC_EnableIRQ(EINT3_IRQn);
}

uint32_t ObcKeypad::getKeys()
{
	IO* x[5] = {&x0, &x1, &x2, &x3, &x4};
	Input* y[4] = {&y0, &y1, &y2, &y3};

	uint32_t keys = 0;
	uint32_t bit = 0;

	//disable all column outputs
	for(uint8_t ix = 0; ix < 5; ix++)
		*x[ix] = true;

	//enable column outputs one at a time and read row inputs
	for(uint8_t ix = 0; ix < 5; ix++)
	{
		*x[ix] = false;
// 		while(*x[ix]);
		for(uint8_t iy = 0; iy < 4; iy++)
		{
			if(!*y[iy])
				keys += 1<<bit;
			bit++;
		}
		*x[ix] = true;
// 		while(!*x[ix]);
	}

	//enable all column outputs again so next keypress can trigger an interrupt
	for(uint8_t ix = 0; ix < 5; ix++)
		*x[ix] = false;

	return keys;
}

void ObcKeypad::scan()
{
	uint32_t key = getKeys(); //TODO support multiple keys

	switch(key)
	{
		case BUTTON_1000_MASK:
			call(BUTTON_1000);
			break;
		case BUTTON_100_MASK:
			call(BUTTON_100);
			break;
		case BUTTON_10_MASK:
			call(BUTTON_10);
			break;
		case BUTTON_1_MASK:
			call(BUTTON_1);
			break;
		case BUTTON_CONSUM_MASK:
			call(BUTTON_CONSUM);
			break;
		case BUTTON_TEMP_MASK:
			call(BUTTON_TEMP);
			break;
		case BUTTON_SPEED_MASK:
			call(BUTTON_SPEED);
			break;
		case BUTTON_DIST_MASK:
			call(BUTTON_DIST);
			break;
		case BUTTON_CHECK_MASK:
			call(BUTTON_CHECK);
			break;
		case BUTTON_RANGE_MASK:
			call(BUTTON_RANGE);
			break;
		case BUTTON_CODE_MASK:
			call(BUTTON_CODE);
			break;
		case BUTTON_LIMIT_MASK:
			call(BUTTON_LIMIT);
			break;
		case BUTTON_TIMER_MASK:
			call(BUTTON_TIMER);
			break;
		case BUTTON_KMMLS_MASK:
			call(BUTTON_KMMLS);
			break;
		case BUTTON_CLOCK_MASK:
			call(BUTTON_CLOCK);
			break;
		case BUTTON_DATE_MASK:
			call(BUTTON_DATE);
			break;
		case BUTTON_MEMO_MASK:
			call(BUTTON_MEMO);
			break;
		case BUTTON_SET_MASK:
			call(BUTTON_SET);
			break;
			
	}
}

void ObcKeypad::interruptHandler()
{
	if((GPIO_GetIntStatus(y0.getPort(), y0.getPin(), 1)) || (GPIO_GetIntStatus(y1.getPort(), y1.getPin(), 1)) || (GPIO_GetIntStatus(y2.getPort(), y2.getPin(), 1)) || (GPIO_GetIntStatus(y3.getPort(), y3.getPin(), 1)))
	{
		if(debounce.read_ms() >= 100)
			scan();
		debounce.start();
		GPIO_ClearInt(y0.getPort(), (1<<y0.getPin()));
		GPIO_ClearInt(y1.getPort(), (1<<y1.getPin()));
		GPIO_ClearInt(y2.getPort(), (1<<y2.getPin()));
		GPIO_ClearInt(y3.getPort(), (1<<y3.getPin()));
	}
	if((GPIO_GetIntStatus(y0.getPort(), y0.getPin(), 0)) || (GPIO_GetIntStatus(y1.getPort(), y1.getPin(), 0)) || (GPIO_GetIntStatus(y2.getPort(), y2.getPin(), 0)) || (GPIO_GetIntStatus(y3.getPort(), y3.getPin(), 0)))
	{
		debounce.start();
		GPIO_ClearInt(y0.getPort(), (1<<y0.getPin()));
		GPIO_ClearInt(y1.getPort(), (1<<y1.getPin()));
		GPIO_ClearInt(y2.getPort(), (1<<y2.getPin()));
		GPIO_ClearInt(y3.getPort(), (1<<y3.getPin()));
	}
}

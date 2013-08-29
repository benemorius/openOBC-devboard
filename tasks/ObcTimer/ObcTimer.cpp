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

#include "ObcTimer.h"
#include <ObcUI.h>
#include <cstdlib>

using namespace ObcTimerState;

ObcTimer::ObcTimer(OpenOBC& obc) : ObcUITask(obc), timer(obc.interruptManager)
{
	setDisplay("ObcTimer");
	state = Inactive;
	timedValue = strtof(obc.config->getValueByNameWithDefault("ObcTimerTimedValue", "%f", 0).c_str(), NULL);
	setDisplayRefreshPeriod(0.100);
}

ObcTimer::~ObcTimer()
{

}

void ObcTimer::wake()
{
	runTask();
}

void ObcTimer::sleep()
{
	obc.config->setValueByName("ObcTimerTimedValue", "%f", timedValue);
}

void ObcTimer::runTask()
{
	if(state == Armed && obc.speed->getKmh() != 0)
	{
		timer.start();
		state = Timing;
	}
	
	if(state == Timing && obc.speed->getKmh() >= 100)
	{
		timedValue = timer.read();
		state = Inactive;
	}
	
	if(state == Timing && timer.read() >= 100)
	{
		state = Inactive;
	}
	
	if(state == Inactive)
	{
// 		setDisplay("0-100 % 3u km/h % 2.1f", 0, timedValue);
		//work around broken printf float implementation
		uint32_t seconds = timedValue;
		uint32_t tenths = (timedValue - seconds) * 10;
		setDisplay("0-100 % 3u km/h % 2u.%u", 0, seconds, tenths);
	}
	else if(state == Armed)
	{
		setDisplay("0-100 % 3u km/h % 2.1f", 0, 0);
	}
	else if(state == Timing)
	{
// 		setDisplay("0-100 % 3u km/h % 2.1f", (uint32_t)(obc.speed->getKmh()), timer.read());
		//work around broken printf float implementation
		float currentTimerValue = timer.read();
		uint32_t seconds = currentTimerValue;
		uint32_t tenths = (currentTimerValue - seconds) * 10;
		setDisplay("0-100 % 3u km/h % 2u.%u", (uint32_t)(obc.speed->getKmh()), seconds, tenths);
	}
	
	if(state == Armed)
		obc.timerLed->on();
	else
		obc.timerLed->off();
	//TODO flash led while Timing
}

void ObcTimer::buttonHandler(ObcUITaskFocus::type focus, uint32_t buttonMask)
{
	if(focus == ObcUITaskFocus::background)
	{
		if(buttonMask == BUTTON_TIMER_MASK)
			obc.ui->setActiveTask(this);
		return;
	}
	
	if(buttonMask == BUTTON_SET_MASK)
	{
		if(state == Inactive && obc.speed->getKmh() < 10)
		{
			state = Armed;
		}
		else if(state == Armed)
		{
			state = Inactive;
		}
		else if(state == Timing)
		{
			state = Inactive;
		}
	}
}

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

#include "ObcClock.h"
#include <ObcUI.h>

using namespace ObcClockState;


ObcClock::ObcClock(OpenOBC& obc) : ObcUITask(obc)
{
	obc.ui->setActiveTaskClock(this);
}

ObcClock::~ObcClock()
{

}

void ObcClock::wake()
{
	std::string configState = obc.config->getValueByNameWithDefault("ObcClockState", "Clock");
	if(configState == "Clock")
		state = Clock;
	else if(configState == "Date")
		state = Date;
	runTask();
}

void ObcClock::sleep()
{
	if(state == Clock)
		obc.config->setValueByName("ObcClockState", "Clock");
	else if(state == Date)
		obc.config->setValueByName("ObcClockState", "Date");
}

void ObcClock::runTask()
{
	if(state == Clock || state == ClockSet)
	{
		setDisplayClock("%02i%02i", obc.rtc->getHour(), obc.rtc->getMinute());
		obc.lcd->setSymbol(ObcLcdSymbols::TopDot, true);
		obc.lcd->setSymbol(ObcLcdSymbols::BottomDot, true);
		obc.lcd->setSymbol(ObcLcdSymbols::Periods, false);
	}
	else if(state == Date || state == DateSet)
	{
		setDisplayClock("%02i%02i", obc.rtc->getMonth(), obc.rtc->getDay());
		obc.lcd->setSymbol(ObcLcdSymbols::TopDot, false);
		obc.lcd->setSymbol(ObcLcdSymbols::BottomDot, false);
		obc.lcd->setSymbol(ObcLcdSymbols::Periods, true);
	}
	else if(state == YearSet)
	{
		setDisplayClock("%04u", obc.rtc->getYear());
		obc.lcd->setSymbol(ObcLcdSymbols::TopDot, false);
		obc.lcd->setSymbol(ObcLcdSymbols::BottomDot, false);
		obc.lcd->setSymbol(ObcLcdSymbols::Periods, false);
	}
	
	if(state == ClockSet)
	{
		setDisplay("set clock");
	}
	else if(state == DateSet)
	{
		setDisplay("set date");
	}
	else if(state == YearSet)
	{
		setDisplay("set year");
	}
}

void ObcClock::buttonHandler(ObcUITaskFocus::type focus, uint32_t buttonMask)
{
	if(obc.ui->getActiveTaskClock() == this)
	{

		if(state == ClockSet && obc.ui->getActiveTask() == this)
		{
			if(buttonMask == BUTTON_SET_MASK)
			{
				state = Clock;
				obc.ui->popActiveTask(this);
			}
			else if(buttonMask == BUTTON_1000_MASK)
			{
				int hour = obc.rtc->getHour() + 10;
				if(hour >= 24)
					while(hour >= 10)
						hour -= 10;
					obc.rtc->setHour(hour);
			}
			else if(buttonMask == BUTTON_100_MASK)
			{
				obc.rtc->setHour(obc.rtc->getHour() + 1);
			}
			else if(buttonMask == BUTTON_10_MASK)
			{
				int minute = obc.rtc->getMinute() + 10;
				if(minute >= 60)
					while(minute >= 10)
						minute -= 10;
					obc.rtc->setMinute(minute);
			}
			else if(buttonMask == BUTTON_1_MASK)
			{
				obc.rtc->setMinute(obc.rtc->getMinute() + 1);
				
			}
		}
		else if(state == DateSet && obc.ui->getActiveTask() == this)
		{
			if(buttonMask == BUTTON_SET_MASK)
			{
				state = YearSet;
			}
			else if(buttonMask == BUTTON_1000_MASK)
			{
				int month = obc.rtc->getMonth() + 10;
				if(month >= 13)
					while(month >= 10)
						month -= 10;
					if(month == 0)
						month = 10;
					obc.rtc->setMonth(month);
			}
			else if(buttonMask == BUTTON_100_MASK)
			{
				obc.rtc->setMonth(obc.rtc->getMonth() + 1);
			}
			else if(buttonMask == BUTTON_10_MASK)
			{
				int day = obc.rtc->getDay() + 10;
				if(day >= 32)
					while(day >= 10)
						day -= 10;
					if(day == 0)
						day = 10;
					obc.rtc->setDay(day);
			}
			else if(buttonMask == BUTTON_1_MASK)
			{
				obc.rtc->setDay(obc.rtc->getDay() + 1);
			}
		}
		else if(state == YearSet && obc.ui->getActiveTask() == this)
		{
			uint16_t year = obc.rtc->getYear();
			if(buttonMask == BUTTON_SET_MASK)
			{
				state = Date;
				obc.ui->popActiveTask(this);
			}
			else if(buttonMask == BUTTON_1000_MASK)
			{
				if((year % 10000) + 1000 >= 10000)
					year -= 10000;
				obc.rtc->setYear(year + 1000);
			}
			else if(buttonMask == BUTTON_100_MASK)
			{
				if((year % 1000) + 100 >= 1000)
					year -= 1000;
				obc.rtc->setYear(year + 100);
			}
			else if(buttonMask == BUTTON_10_MASK)
			{
				if((year % 100) + 10 >= 100)
					year -= 100;
				obc.rtc->setYear(year + 10);
			}
			else if(buttonMask == BUTTON_1_MASK)
			{
				if((year % 10) + 1 >= 10)
					year -= 10;
				obc.rtc->setYear(year + 1);
			}
		}
		else
		{
			if(buttonMask == BUTTON_CLOCK_MASK)
				state = Clock;
			else if(buttonMask == BUTTON_DATE_MASK)
				state = Date;
			else if(buttonMask == (BUTTON_SET_MASK | BUTTON_CLOCK_MASK))
			{
				state = ClockSet;
				obc.ui->setActiveTask(this);
				
			}
			else if(buttonMask == (BUTTON_SET_MASK | BUTTON_DATE_MASK))
			{
				state = DateSet;
				obc.ui->setActiveTask(this);
			}
		}
	}

}

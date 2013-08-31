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

#include "ObcLimit.h"
#include <ObcUI.h>
#include <cstdlib>

using namespace ObcLimitState;

ObcLimit::ObcLimit(OpenOBC& obc) : ObcUITask(obc)
{
	hasWarned = false;
	limitKmh = atof(obc.config->getValueByName("ObcLimitKmh").c_str());
	if(obc.config->getValueByName("ObcLimitState") == "LimitActive")
	{
		state = LimitActive;
		obc.limitLed->on();
		if(obc.ui->getMeasurementSystem() == ObcUIMeasurementSystem::Metric)
			setDisplay("limit % 3u kmh", (uint32_t)limitKmh);
		else if(obc.ui->getMeasurementSystem() == ObcUIMeasurementSystem::Imperial)
			setDisplay("limit % 3u mph", (uint32_t)(limitKmh * 0.621371));
		else if(obc.ui->getMeasurementSystem() == ObcUIMeasurementSystem::Both)
			setDisplay("limit % 3ukmh % 3umph", (uint32_t)limitKmh, (uint32_t)(limitKmh * 0.621371));
	}
	else
	{
		state = LimitInactive;
		obc.limitLed->off();
		setDisplay("limit inactive");
	}
}

ObcLimit::~ObcLimit()
{

}

void ObcLimit::runTask()
{
	if(state == LimitActive)
	{
		float kmh = obc.speed->getKmh();
		if(kmh >= limitKmh && !hasWarned)
		{
			DEBUG("over limit: % 3.0f kmh (limit % 3.0f)\r\n", kmh, limitKmh);
			obc.ui->setActiveTask(this, 5);
			obc.chime0->on();
			obc.ui->callback.addCallback(obc.chime0, &IO::off, 100);
			hasWarned = true;
			//TODO flash Limit LED
		}
		else if(kmh < limitKmh && hasWarned)
		{
			hasWarned = false;
			obc.chime0->off();
			//TODO stop flashing limit LED
		}
	}
}

void ObcLimit::buttonHandler(ObcUITaskFocus::type focus, uint32_t buttonMask)
{
	if(focus == ObcUITaskFocus::background)
	{
		if(buttonMask == BUTTON_LIMIT_MASK)
			obc.ui->setActiveTask(this);
		return;
	}
	
	if(state == LimitInactive)
	{
		if(buttonMask == BUTTON_SET_MASK)
		{
			state = LimitSet;
			limitKmhSet = limitKmh;
		}
	}
	
	else if(state == LimitSet)
	{
		if(buttonMask == BUTTON_SET_MASK)
		{
			state = LimitActive;
			limitKmh = limitKmhSet;
			obc.limitLed->on();
			if(obc.ui->getMeasurementSystem() == ObcUIMeasurementSystem::Metric)
				setDisplay("limit % 3u kmh", (uint32_t)limitKmh);
			else if(obc.ui->getMeasurementSystem() == ObcUIMeasurementSystem::Imperial)
				setDisplay("limit % 3u mph", (uint32_t)(limitKmh * 0.621371));
			else if(obc.ui->getMeasurementSystem() == ObcUIMeasurementSystem::Both)
				setDisplay("limit % 3ukmh % 3umph", (uint32_t)limitKmh, (uint32_t)(limitKmh * 0.621371));
		}
		else if(buttonMask == BUTTON_LIMIT_MASK)
		{
			state = LimitInactive;
			obc.limitLed->off();
			setDisplay("limit inactive");
		}
	}
	
	else if(state == LimitActive)
	{
		if(buttonMask == BUTTON_SET_MASK)
		{
			state = LimitSet;
			limitKmhSet = limitKmh;
		}
	}
	
	//TODO fix very bad handling of miles and overflows
	if((state == LimitSet) && (buttonMask & (BUTTON_1000_MASK | BUTTON_100_MASK | BUTTON_10_MASK | BUTTON_1_MASK | BUTTON_SET_MASK)))
	{
		if(buttonMask == BUTTON_1000_MASK)
		{
			if(obc.ui->getMeasurementSystem() == ObcUIMeasurementSystem::Imperial)
				limitKmhSet += 1000 / 0.621371;
			else
				limitKmhSet += 1000;
		}
		if(buttonMask == BUTTON_100_MASK)
		{
			if(obc.ui->getMeasurementSystem() == ObcUIMeasurementSystem::Imperial)
				limitKmhSet += 100 / 0.621371;
			else
				limitKmhSet += 100;
		}
		if(buttonMask == BUTTON_10_MASK)
		{
			if(obc.ui->getMeasurementSystem() == ObcUIMeasurementSystem::Imperial)
				limitKmhSet += 10 / 0.621371;
			else
				limitKmhSet += 10;
		}
		if(buttonMask == BUTTON_1_MASK)
		{
			if(obc.ui->getMeasurementSystem() == ObcUIMeasurementSystem::Imperial)
				limitKmhSet += 1 / 0.621371;
			else
				limitKmhSet += 1;
		}
		if(obc.ui->getMeasurementSystem() == ObcUIMeasurementSystem::Imperial)
			limitKmhSet = (int32_t)limitKmhSet % (uint32_t)(1000 / 0.621371);
		else
			limitKmhSet = (int32_t)limitKmhSet % 1000;
		
		if(obc.ui->getMeasurementSystem() == ObcUIMeasurementSystem::Metric)
			setDisplay("set limit: % 3u kmh", (uint32_t)limitKmhSet);
		else if(obc.ui->getMeasurementSystem() == ObcUIMeasurementSystem::Imperial)
			setDisplay("set limit: % 3u mph", (uint32_t)(limitKmhSet * 0.621371));
		else if(obc.ui->getMeasurementSystem() == ObcUIMeasurementSystem::Both)
			setDisplay("set:  % 3ukmh % 3umph", (uint32_t)limitKmhSet, (uint32_t)(limitKmhSet * 0.621371));
	}
}

void ObcLimit::sleep()
{
	char* buf = new char[4];
	snprintf(buf, 4, "%.0f", limitKmh);
    obc.config->setValueByName("ObcLimitKmh", buf);
	delete[] buf;
	
	if(state == LimitActive)
		obc.ui->config.setValueByName("ObcLimitState", "LimitActive");
	else
		obc.ui->config.setValueByName("ObcLimitState", "LimitInactive");
}

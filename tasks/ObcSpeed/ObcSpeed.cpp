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

#include "ObcSpeed.h"
#include <ObcUI.h>
#include <cstdlib>

ObcSpeed::ObcSpeed(OpenOBC& obc) : ObcUITask(obc)
{
	averageSpeedKmh = strtof(obc.config->getValueByNameWithDefault("ObcSpeedAverageKmh", "%f", 0).c_str(), NULL);
	averageSeconds = 300;
}

ObcSpeed::~ObcSpeed()
{

}

void ObcSpeed::wake()
{
	runTask();
}

void ObcSpeed::runTask()
{
	float currentSpeedKmh = obc.speed->getKmh();
	if(averageSpeedKmh == 0)
		averageSpeedKmh = currentSpeedKmh;
	static Timer timer(obc.interruptManager);
	if(timer.read() >= 1)
	{
		timer.start();
		float alpha = 1.0 / averageSeconds;
		averageSpeedKmh = alpha * currentSpeedKmh + (1.0 - alpha) * averageSpeedKmh;
	}
	
	obc.averageKmh = averageSpeedKmh;
	
	if(obc.ui->getMeasurementSystem() == ObcUIMeasurementSystem::Metric)
		setDisplay("% 3u km/h % 3u avg", (uint32_t)currentSpeedKmh, (uint32_t)averageSpeedKmh);
	else if(obc.ui->getMeasurementSystem() == ObcUIMeasurementSystem::Imperial)
		setDisplay("% 3u mph % 3u avg", (uint32_t)(currentSpeedKmh * 0.621371), (uint32_t)(averageSpeedKmh * 0.621371));
	else if(obc.ui->getMeasurementSystem() == ObcUIMeasurementSystem::Both)
		setDisplay("avg: % 3u kmh % 3u mph", (uint32_t)averageSpeedKmh, (uint32_t)(averageSpeedKmh * 0.621371));
	
}

void ObcSpeed::buttonHandler(ObcUITaskFocus::type focus, uint32_t buttonMask)
{
	if(focus == ObcUITaskFocus::background)
	{
		if(buttonMask == BUTTON_SPEED_MASK)
			obc.ui->setActiveTask(this);
		return;
	}
	
	if(buttonMask == BUTTON_SET_MASK)
	{
		averageSpeedKmh = obc.speed->getKmh();
	}
}

void ObcSpeed::sleep()
{
    obc.config->setValueByName("ObcSpeedAverageKmh", "%f", averageSpeedKmh);
}

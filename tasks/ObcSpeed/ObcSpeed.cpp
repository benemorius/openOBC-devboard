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
	averageSpeedKph = strtof(obc.config->getValueByName("ObcSpeedAverageKph").c_str(), NULL);
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
	uint32_t currentSpeedKmh = obc.speed->getKmh();
	
	
	
	
	
	
	
	if(obc.ui->getMeasurementSystem() == ObcUIMeasurementSystem::Metric)
		setDisplay("% 3u Km/h % 3u avg", currentSpeedKmh, averageSpeedKph);
	else if(obc.ui->getMeasurementSystem() == ObcUIMeasurementSystem::Imperial)
		setDisplay("% 3u mph % 3u avg", currentSpeedKmh * 0.621371, averageSpeedKph * 0.621371);
	else if(obc.ui->getMeasurementSystem() == ObcUIMeasurementSystem::Both)
		setDisplay("avg: % 3u Kmh % 3u mph", averageSpeedKph, averageSpeedKph * 0.621371);
	
}

void ObcSpeed::buttonHandler(ObcUITaskFocus::type focus, uint32_t buttonMask)
{
	if(focus == ObcUITaskFocus::background)
	{
		if(buttonMask == BUTTON_SPEED_MASK)
			obc.ui->setActiveTask(this);
		return;
	}
	
	
	
}

void ObcSpeed::sleep()
{
    obc.config->setValueByName("ObcSpeedAverageKph", "%.4f", averageSpeedKph);
}

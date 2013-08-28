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

#include "ObcConsum.h"
#include <ObcUI.h>
#include <cstdlib>

ObcConsum::ObcConsum(OpenOBC& obc) : ObcUITask(obc)
{
	setDisplay("ObcConsum");
	averageLitresPer100km = strtof(obc.config->getValueByName("ObcConsumAverageLitresPer100km").c_str(), NULL);
	averageFuelConsumptionSeconds = strtoul(obc.config->getValueByName("ObcConsumAverageFuelConsumptionSeconds").c_str(), NULL, 0);
}

ObcConsum::~ObcConsum()
{

}

void ObcConsum::wake()
{
	runTask();
}

void ObcConsum::sleep()
{
	obc.config->setValueByName("ObcConsumAverageLitresPer100km", "%f", averageLitresPer100km);;
	obc.config->setValueByName("ObcConsumAverageFuelConsumptionSeconds", "%u", averageFuelConsumptionSeconds);;
}

void ObcConsum::runTask()
{
	static Timer averageLitresPer100kmTimer(obc.interruptManager);
	
	float kilometresPerHour = obc.speed->getKmh();
	if(averageLitresPer100kmTimer.read_ms() >= 1000 && kilometresPerHour > 1)
	{
		averageLitresPer100kmTimer.start();
		float litresPerHour = 0.2449 * 6 * 60 * obc.fuelCons->getDutyCycle();
		float litresPer100km = litresPerHour / kilometresPerHour * 100;
		if(averageFuelConsumptionSeconds > 0)
			averageLitresPer100km = (averageLitresPer100km * (averageFuelConsumptionSeconds - 1) + litresPer100km) / averageFuelConsumptionSeconds;
		else
			averageLitresPer100km = litresPer100km;
		averageFuelConsumptionSeconds++;
		DEBUG("new average fuel consumption is %.1fmpg (%.1fmpg %.1fmph %.1fkm/h) with %i seconds\r\n", 235.214f / averageLitresPer100km, 235.214f / litresPer100km, kilometresPerHour / 1.609f, kilometresPerHour, averageFuelConsumptionSeconds);
		obc.averageLitresPer100km = averageLitresPer100km;
	}
	
	if(obc.ui->getMeasurementSystem() == ObcUIMeasurementSystem::Metric)
		setDisplay("avg %2.1f L/100km", averageLitresPer100km);
	else if(obc.ui->getMeasurementSystem() == ObcUIMeasurementSystem::Imperial)
		setDisplay("avg %2.1f mpg", 235.214f / averageLitresPer100km);
	else if(obc.ui->getMeasurementSystem() == ObcUIMeasurementSystem::Both)
		setDisplay("%2.1fL/100km %2.1fmpg", averageLitresPer100km, 235.214f / averageLitresPer100km);
}

void ObcConsum::buttonHandler(ObcUITaskFocus::type focus, uint32_t buttonMask)
{
	if(focus == ObcUITaskFocus::background)
	{
		if(buttonMask == BUTTON_CONSUM_MASK)
			obc.ui->setActiveTask(this);
		return;
	}
	
	if(buttonMask == BUTTON_SET_MASK)
	{
		averageFuelConsumptionSeconds = 0;
		averageLitresPer100km = 0;
	}
}

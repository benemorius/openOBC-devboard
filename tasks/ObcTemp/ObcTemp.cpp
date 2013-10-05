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

#include "ObcTemp.h"
#include <ObcUI.h>
#include <cmath>
#include <cstdlib>

using namespace ObcTempState;

ObcTemp::ObcTemp(OpenOBC& obc) : ObcUITask(obc)
{
	setDisplay("ObcTemp");
	
	std::string configState = obc.config->getValueByNameWithDefault("ObcTempState", "TempExt");
	if(configState == "TempCoolant")
		state = TempCoolant;
	else if(configState == "TempExt")
		state = TempExt;
	
	coolantWarningTemp = strtoul(obc.config->getValueByNameWithDefault("ObcTempCoolantWarningTemp", "100").c_str(), NULL, 0);
}

ObcTemp::~ObcTemp()
{

}

void ObcTemp::wake()
{
	runTask();
}

void ObcTemp::sleep()
{
	obc.config->setValueByName("ObcTempCoolantWarningTemp", "%i", coolantWarningTemp);
	if(state == TempCoolant)
		obc.config->setValueByName("ObcTempState", "TempCoolant");
	else
		obc.config->setValueByName("ObcTempState", "TempExt");
}

void ObcTemp::runTask()
{
	if(state == TempExt)
	{
		float voltage = obc.temperature->read();
		float resistance = (10000 * voltage) / (REFERENCE_VOLTAGE - voltage);
		float temperature = 1.0 / ((1.0 / 298.15) + (1.0/3950) * log(resistance / 4700)) - 273.15f;
		if(obc.ui->getMeasurementSystem() == ObcUIMeasurementSystem::Both)
			setDisplay("ext. % 2.1fC % 3.0fF", temperature, temperature * 1.78 + 32);
		if(obc.ui->getMeasurementSystem() == ObcUIMeasurementSystem::Metric)
			setDisplay("exterior % 2.1fC", temperature);
		if(obc.ui->getMeasurementSystem() == ObcUIMeasurementSystem::Imperial)
			setDisplay("exterior % 3.0fF", temperature * 1.78 + 32);
	}
	else if(state == TempCoolant)
	{
		if(obc.ui->getMeasurementSystem() == ObcUIMeasurementSystem::Both)
			setDisplay("coolant % 2.0fC %.0fF", obc.coolantTemperature, obc.coolantTemperature * 1.78 + 32);
		if(obc.ui->getMeasurementSystem() == ObcUIMeasurementSystem::Metric)
			setDisplay("coolant temp % 2.0fC", obc.coolantTemperature);
		if(obc.ui->getMeasurementSystem() == ObcUIMeasurementSystem::Imperial)
			setDisplay("coolant temp % 3.0fF", obc.coolantTemperature * 1.78 + 32);
	}
	else if(state == TempCoolantWarningSet)
	{
		setDisplay("set warning % 3iC", coolantWarningTempSet);
	}
	
	static Timer coolantWarningTimer;
	static bool hasWarned;
	if(obc.coolantTemperature >= coolantWarningTemp && !hasWarned)
	{
		coolantWarningTimer.start();
		hasWarned = true;
		obc.ui->setActiveTask(this, 5);
		state = TempCoolant;
		obc.ccmLight->on();
		obc.ui->callback.addCallback(obc.ccmLight, &IO::off, 4000);
		obc.chime1->on();
		obc.ui->callback.addCallback(obc.chime1, &IO::off, 100);
	}
	else if(obc.coolantTemperature >= coolantWarningTemp)
	{
		if(coolantWarningTimer.read_ms() >= 5000)
		{
			coolantWarningTimer.start();
			obc.ccmLight->on();
			obc.ui->callback.addCallback(obc.ccmLight, &IO::off, 4000);
			obc.chime1->on();
			obc.ui->callback.addCallback(obc.chime1, &IO::off, 100);
		}
	}
	else
	{
		hasWarned = false;
	}
}

void ObcTemp::buttonHandler(ObcUITaskFocus::type focus, uint32_t buttonMask)
{
	if(focus == ObcUITaskFocus::background)
	{
		if(buttonMask == BUTTON_TEMP_MASK)
			obc.ui->setActiveTask(this);
		return;
	}
	
	if(buttonMask == BUTTON_TEMP_MASK)
	{
		if(state == TempExt)
			state = TempCoolant;
		else if(state == TempCoolant)
			state = TempExt;
	}
	
	else if(buttonMask == BUTTON_SET_MASK)
	{
		if(state == TempCoolant)
		{
			coolantWarningTempSet = coolantWarningTemp;
			state = TempCoolantWarningSet;
		}
		else
		{
			coolantWarningTemp = coolantWarningTempSet;
			state = TempCoolant;
		}
	}
	
	else if((state == TempCoolantWarningSet) && (buttonMask & (BUTTON_1000_MASK | BUTTON_100_MASK | BUTTON_10_MASK | BUTTON_1_MASK)))
	{
		if(buttonMask == BUTTON_1000_MASK)
		{
			coolantWarningTempSet += 1000;
		}
		if(buttonMask == BUTTON_100_MASK)
		{
			coolantWarningTempSet += 100;
		}
		if(buttonMask == BUTTON_10_MASK)
		{
			coolantWarningTempSet += 10;
		}
		if(buttonMask == BUTTON_1_MASK)
		{
			coolantWarningTempSet += 1;
		}
		coolantWarningTempSet %= 1000;
	}
}

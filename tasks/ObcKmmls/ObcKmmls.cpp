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

#include "ObcKmmls.h"
#include <ObcUI.h>

ObcKmmls::ObcKmmls(OpenOBC& obc) : ObcUITask(obc)
{

}

ObcKmmls::~ObcKmmls()
{

}

void ObcKmmls::wake()
{
	runTask();
}

void ObcKmmls::runTask()
{
	ObcUIMeasurementSystem::system measurementSystem = obc.ui->getMeasurementSystem();
	if(measurementSystem == ObcUIMeasurementSystem::Metric)
		setDisplay("metric");
	else if(measurementSystem == ObcUIMeasurementSystem::Imperial)
		setDisplay("imperial");
	else if(measurementSystem == ObcUIMeasurementSystem::Both)
		setDisplay("metric / imperial");
}

void ObcKmmls::buttonHandler(ObcUITaskFocus::type focus, uint32_t buttonMask)
{
	if(buttonMask == BUTTON_KMMLS_MASK)
	{
		obc.ui->setActiveTask(this, 2);
		if(focus == ObcUITaskFocus::active)
		{
			ObcUIMeasurementSystem::system measurementSystem = obc.ui->getMeasurementSystem();
			if(measurementSystem == ObcUIMeasurementSystem::Both)
				measurementSystem = ObcUIMeasurementSystem::Metric;
			else if(measurementSystem == ObcUIMeasurementSystem::Metric)
				measurementSystem = ObcUIMeasurementSystem::Imperial;
			else if(measurementSystem == ObcUIMeasurementSystem::Imperial)
				measurementSystem = ObcUIMeasurementSystem::Both;
			obc.ui->setMeasurementSystem(measurementSystem);
		}
	}
}

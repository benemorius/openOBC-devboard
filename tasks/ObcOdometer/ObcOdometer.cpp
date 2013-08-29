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

#include "ObcOdometer.h"
#include <ObcUI.h>
#include <cstdlib>

ObcOdometer::ObcOdometer(OpenOBC& obc) : ObcUITask(obc), timer(obc.interruptManager)
{
	setDisplay("ObcOdometer");
	currentKm = strtof(obc.config->getValueByNameWithDefault("ObcOdometerCurrentKm", "%f", 0).c_str(), NULL);
}

ObcOdometer::~ObcOdometer()
{

}

void ObcOdometer::wake()
{
	runTask();
}

void ObcOdometer::sleep()
{
	obc.config->setValueByName("ObcOdometerCurrentKm", "%f", currentKm);
}


void ObcOdometer::runTask()
{
	if(timer.read() >= 1)
	{
		timer.start();
		currentKm += obc.speed->getKmh() / 60 / 60;
	}
	obc.currentKm = currentKm;
}

void ObcOdometer::buttonHandler(ObcUITaskFocus::type focus, uint32_t buttonMask)
{

}

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

#include "ObcDist.h"
#include <ObcUI.h>

using namespace ObcDistState;

ObcDist::ObcDist(OpenOBC& obc) : ObcUITask(obc)
{
	setDisplay("ObcDist");
	state = Voltage;
}

ObcDist::~ObcDist()
{

}

void ObcDist::wake()
{
	runTask();
}

void ObcDist::runTask()
{
	if(state == Voltage)
		setDisplay("%.2fV %.2fV %.2fV", obc.batteryVoltage->read(), obc.analogIn1->read(), obc.analogIn2->read());
	else if(state == FreeMem)
		setDisplay("free memory: %i", get_mem_free());
}

void ObcDist::buttonHandler(ObcUITaskFocus::type focus, uint32_t buttonMask)
{
	if(focus == ObcUITaskFocus::background)
	{
		if(buttonMask == BUTTON_DIST_MASK)
			obc.ui->setActiveTask(this);
		return;
	}
	
	if(buttonMask == BUTTON_DIST_MASK)
	{
		if(state == Voltage)
			state = FreeMem;
		else if(state == FreeMem)
			state = Voltage;
	}
}

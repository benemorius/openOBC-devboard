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


#include "ObcUITask.h"

ObcUITask::ObcUITask(OpenOBC& obc) : obc(obc)
{
	setDisplayRefreshPeriod(0.100);
	_isActive = false;
	activeTaskTimeout = 0;
}

ObcUITask::~ObcUITask()
{

}

void ObcUITask::registerButton(ObcUITaskFocus::type focus, uint32_t buttonMask)
{
	//FIXME TODO
}

void ObcUITask::unregisterButton(ObcUITaskFocus::type focus, uint32_t buttonMask)
{
	//FIXME TODO
}

void ObcUITask::setDisplay(char* format, ...)
{
	va_list args;
	va_start(args, format);
	vsnprintf(displayBuffer, sizeof(displayBuffer), format, args);
	va_end(args);
}

void ObcUITask::setDisplayClock(char* format, ... )
{
	va_list args;
	va_start(args, format);
	vsnprintf(displayClockBuffer, sizeof(displayClockBuffer), format, args);
	va_end(args);
}

void ObcUITask::createButtonEvent(ObcUITaskFocus::type focus, uint32_t buttonMask)
{
	if(focus == ObcUITaskFocus::active)
		buttonEventsActive.push_back(buttonMask);
	else if(focus == ObcUITaskFocus::background)
		buttonEventsBackground.push_back(buttonMask);
}

void ObcUITask::runButtonEvents()
{
	for(std::vector<uint32_t>::iterator buttonMask = buttonEventsActive.begin(); buttonMask != buttonEventsActive.end(); buttonEventsActive.erase(buttonMask))
	{
		buttonHandler(ObcUITaskFocus::active, *buttonMask);
	}
	for(std::vector<uint32_t>::iterator buttonMask = buttonEventsBackground.begin(); buttonMask != buttonEventsBackground.end(); buttonEventsBackground.erase(buttonMask))
	{
		buttonHandler(ObcUITaskFocus::background, *buttonMask);
	}
}

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


#include "ObcUI.h"

ObcUI::ObcUI(ObcLcd& lcd) : lcd(lcd)
{
	activeTask = NULL;
}

ObcUI::~ObcUI()
{

}

void ObcUI::task()
{
	
	//run all queued button events
	FunctionPointer<void>* event = NULL;
	while(event = buttonEvents.back())
	{
		event->call();
	}

	
	//run all tasks
	ObcUITask* task = NULL;
	while(task = tasks.back())
	{
		task->runTask();
		
		//...or run all queued button events here
		task->runEvents();
	}
	
	//update display
	if(activeTask)
	{
		lcd.printf("%s", activeTask->getDisplay().c_str());
	}
}

void ObcUI::handleButtonEvent()
{
	//determine if any tasks are hooked to the pushed button
	//and whether the hook is active or background
}

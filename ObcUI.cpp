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
#include "ObcUITask.h"

ObcUI::ObcUI(ObcLcd& lcd, ObcKeypad& keypad) : lcd(lcd), keypad(keypad)
{
	activeTask = NULL;
}

ObcUI::~ObcUI()
{

}

void ObcUI::task()
{
	
// 	//run all queued button events
// 	FunctionPointer<void>* event = NULL;
// 	while(event = buttonEvents.back())
// 	{
// 		event->call();
// 	}

	
	//run all tasks and queued button events
	for(std::vector<ObcUITask*>::iterator task = tasks.begin(); task != tasks.end(); ++task)
	{
		DEBUG("running %i tasks\r\n", tasks.size());
		(*task)->runTask();
// 		(*task)->runEvents();
	}
	
	//update display
	static Timer displayUpdateTimer;
	if(activeTask && (displayUpdateTimer.read() >= activeTask->getDisplayRefreshPeriod()))
	{
		displayUpdateTimer.start();
		lcd.printf("%s", activeTask->getDisplay());
	}
}

void ObcUI::handleButtonEvent()
{
	//determine if any tasks are hooked to the pushed button
	//and whether the hook is active or background
}

void ObcUI::addTask(ObcUITask* task)
{

}

void ObcUI::removeTask(ObcUITask* task)
{

}

void ObcUI::setActiveTask(ObcUITask* task, float forSeconds)
{
	if(task == NULL)
		return;
	task->setActive(true);
	activeTask->setActive(false);
	activeTask = task;
}

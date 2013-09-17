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
#include <algorithm>

ObcUI::ObcUI(ObcLcd& lcd, ObcKeypad& keypad, ConfigFile& config) : lcd(lcd), keypad(keypad), config(config)
{
	activeTask = NULL;
	activeTaskClock = NULL;
	
	if(config.getValueByName("MeasurementSystem") == "Metric")
		measurementSystem = ObcUIMeasurementSystem::Metric;
	else if(config.getValueByName("MeasurementSystem") == "Imperial")
		measurementSystem = ObcUIMeasurementSystem::Imperial;
	else
	{
		config.setValueByName("MeasurementSystem", "Both");
		measurementSystem = ObcUIMeasurementSystem::Both;
	}
}

ObcUI::~ObcUI()
{

}

void ObcUI::task()
{
	callback.task();
	
	//run all tasks and queued button events
	for(std::vector<ObcUITask*>::iterator task = tasks.begin(); task != tasks.end(); ++task)
	{
// 		DEBUG("running %i tasks\r\n", tasks.size());
		(*task)->runButtonEvents();
		(*task)->runTask();
	}
	
	if(activeTask && activeTask->getActiveTaskTimeout() && (activeTaskTimeout.read() >= activeTask->getActiveTaskTimeout()))
	{
// 		DEBUG("task timed out after %.1f seconds\r\n", activeTask->getActiveTaskTimeout());
		activeTask->setActive(false);
		activeTaskList.pop_back();
		if(!activeTaskList.empty())
		{
			activeTask = activeTaskList.back();
			activeTask->setActive(true);
		}
		else
		{
			activeTask = NULL;
			lcd.printf("");
		}
	}
	
	//update display
	static Timer displayUpdateTimer;
	if(activeTask && (displayUpdateTimer.read() >= activeTask->getDisplayRefreshPeriod()))
	{
		displayUpdateTimer.start();
		lcd.printf("%s", activeTask->getDisplay());
	}
	static Timer displayClockUpdateTimer;
	if(activeTaskClock && (displayClockUpdateTimer.read_ms() >= 100))
	{
		displayClockUpdateTimer.start();
		lcd.printfClock("%s", activeTaskClock->getDisplayClock());
	}
}

void ObcUI::addTask(ObcUITask* task)
{
	if(task == NULL)
		return;
	tasks.push_back(task);
}

void ObcUI::removeTask(ObcUITask* task)
{
	for(std::vector<ObcUITask*>::iterator task = tasks.begin(); task != tasks.end(); ++task)
	{
		task = tasks.erase(task);
		break;
	}

	std::deque<ObcUITask*>::iterator found;
	while((found = std::find(activeTaskList.begin(), activeTaskList.end(), task)) != activeTaskList.end())
		activeTaskList.erase(found);
}

void ObcUI::setActiveTask(ObcUITask* task, float forSeconds)
{
	if(task == NULL)
	{
		lcd.printf("");
		activeTask = NULL;
		return;
	}
// 	DEBUG("setting active task\r\n");
	task->setActive(true);
	if(activeTask != NULL)
		activeTask->setActive(false);
	activeTask = task;
	lcd.printf("%s", activeTask->getDisplay());
	
	std::deque<ObcUITask*>::iterator found;
	while((found = std::find(activeTaskList.begin(), activeTaskList.end(), task)) != activeTaskList.end())
		activeTaskList.erase(found);
	activeTaskList.push_back(task);
	activeTask->setActiveTaskTimeout(forSeconds);
	activeTaskTimeout.start();
}

ObcUITask* ObcUI::popActiveTask(ObcUITask* task)
{
	if(activeTaskList.size() == 0)
		return NULL;
		
	if(task == NULL)
	{
		task = activeTaskList.back();
	}
	
	std::deque<ObcUITask*>::iterator found = std::find(activeTaskList.begin(), activeTaskList.end(), task);
	if(found  == activeTaskList.end())
		return NULL;
	
	activeTaskList.erase(found);
	
	if(task == activeTask)
	{
		if(activeTaskList.size() > 0)
		{
			setActiveTask(activeTaskList.back());
		}
		else
		{
			setActiveTask(NULL);
		}
	}
	
	return task;
}

void ObcUI::wake()
{
	for(std::vector<ObcUITask*>::iterator task = tasks.begin(); task != tasks.end(); ++task)
	{
		(*task)->wake();
	}
}

void ObcUI::sleep()
{
	for(std::vector<ObcUITask*>::iterator task = tasks.begin(); task != tasks.end(); ++task)
	{
		(*task)->sleep();
	}
	
	if(measurementSystem == ObcUIMeasurementSystem::Metric)
		config.setValueByName("MeasurementSystem", "Metric");
	else if(measurementSystem == ObcUIMeasurementSystem::Imperial)
		config.setValueByName("MeasurementSystem", "Imperial");
	else if(measurementSystem == ObcUIMeasurementSystem::Both)
		config.setValueByName("MeasurementSystem", "Both");
}

void ObcUI::handleButtonEvent(uint32_t buttonMask)
{
	for(std::vector<ObcUITask*>::iterator task = tasks.begin(); task != tasks.end(); ++task)
	{
		if(*task == activeTask)
		{
			//FIXME TODO check whether the button has been unregistered before creating the event
			(*task)->createButtonEvent(ObcUITaskFocus::active, buttonMask);
		}
		else
		{
			//FIXME TODO check whether the button is registered before creating the event
			(*task)->createButtonEvent(ObcUITaskFocus::background, buttonMask);
		}
	}
}

void ObcUI::registerButton(ObcUITask* task, ObcUITaskFocus::type focus, uint32_t buttonMask)
{
	//FIXME TODO
}

void ObcUI::unregisterButton(ObcUITask* task, ObcUITaskFocus::type focus, uint32_t buttonMask)
{
	//FIXME TODO
}

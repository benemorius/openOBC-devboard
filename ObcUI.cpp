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

ObcUI::ObcUI(ObcLcd& lcd, ObcKeypad& keypad, ConfigFile& config) : lcd(lcd), keypad(keypad), config(config)
{
	activeTask = NULL;
	
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
	
	//FIXME TODO check active task timeout and update activeTask accordingly
	
	//update display
	static Timer displayUpdateTimer;
	if(activeTask && (displayUpdateTimer.read() >= activeTask->getDisplayRefreshPeriod()))
	{
		displayUpdateTimer.start();
		lcd.printf("%s", activeTask->getDisplay());
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
}

void ObcUI::setActiveTask(ObcUITask* task, float forSeconds)
{
	if(task == NULL)
		return;
	task->setActive(true);
	if(activeTask != NULL)
		activeTask->setActive(false);
	activeTask = task;
	lcd.printf("%s", activeTask->getDisplay());
	
	//FIXME TODO do something with forSeconds
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

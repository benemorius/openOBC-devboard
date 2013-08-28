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

#ifndef OBCUI_H
#define OBCUI_H

#include "ObcUITask.h"
#include "ObcLcd.h"
#include "ObcKeypad.h"
#include <vector>
#include <board.h>
#include <Timer.h>

namespace ObcUIMeasurementSystem
{
	enum system {Metric, Imperial, Both};
}

class ObcUI
{

public:
	ObcUI(ObcLcd& lcd, ObcKeypad& keypad, ConfigFile& config);
	~ObcUI();
	
	void handleButtonEvent(uint32_t buttonMask);
	
	void task();
	
	void addTask(ObcUITask* task);
	void removeTask(ObcUITask* task);
	
	void setActiveTask(ObcUITask* task, float forSeconds = 0);
	ObcUITask* getActiveTask() {return activeTask;}
	
	void registerButton(ObcUITask* task, ObcUITaskFocus::type focus, uint32_t buttonMask);
	void unregisterButton(ObcUITask* task, ObcUITaskFocus::type focus, uint32_t buttonMask);
	
	void wake();
	void sleep();
	
	void setMeasurementSystem(ObcUIMeasurementSystem::system system) {measurementSystem = system;}
	ObcUIMeasurementSystem::system getMeasurementSystem() {return measurementSystem;}
	
	Callback callback;
	ConfigFile& config;
	
private:
	ObcLcd& lcd;
	ObcKeypad& keypad;
	std::vector<ObcUITask*> tasks;
	ObcUITask* activeTask;
	ObcUIMeasurementSystem::system measurementSystem;
	Timer activeTaskTimeout;
	std::deque<ObcUITask*> activeTaskList;
};

#endif // OBCUI_H

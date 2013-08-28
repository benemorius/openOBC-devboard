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

#ifndef OBCUITASK_H
#define OBCUITASK_H

#include <stdint.h>
#include <vector>
#include "OpenOBC.h"
#include <stdarg.h>

class OpenOBC;

namespace ObcUITaskFocus {
	enum type {active, background};
}

class ObcUITask
{

public:
	ObcUITask(OpenOBC& obc);
	~ObcUITask();
	
	//required methods
	virtual void runTask() = 0; //mainloop code goes here
	virtual void buttonHandler(ObcUITaskFocus::type focus, uint32_t buttonMask) = 0; //this is called once for each button press
	
	//optional methods
	virtual void wake() {}; //run once on startup before entering main loop
	virtual void sleep() {}; //run once just before switching to sleep state
	
	char* getDisplay() {return displayBuffer;}
	float getDisplayRefreshPeriod() {return displayRefreshPeriod;}
	void setActive(bool isActive) {_isActive = isActive;}
	void runButtonEvents();
	
	void createButtonEvent(ObcUITaskFocus::type focus, uint32_t buttonMask);
	void setActiveTaskTimeout(float seconds) {activeTaskTimeout = seconds;}
	float getActiveTaskTimeout() {return activeTaskTimeout;}
	
protected:
	void registerButton(ObcUITaskFocus::type focus, uint32_t buttonMask);
	void unregisterButton(ObcUITaskFocus::type focus, uint32_t buttonMask);
	void setDisplay(char* format, ...);
	bool isActive() {return _isActive;}
	void setDisplayRefreshPeriod(float seconds) {displayRefreshPeriod = seconds;}

	OpenOBC& obc;

private:
	std::vector<uint32_t> buttonEventsActive;
	std::vector<uint32_t> buttonEventsBackground;
	char displayBuffer[21];
	bool _isActive;
    float displayRefreshPeriod;
	float activeTaskTimeout;
};

#endif // OBCUITASK_H

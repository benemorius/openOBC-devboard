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

#include "ObcCheck.h"
#include <ObcUI.h>

using namespace ObcCCMBits;

ObcCheck::ObcCheck(OpenOBC& obc) : ObcUITask(obc)
{
	registerButton(ObcUITaskFocus::background, BUTTON_CHECK_MASK);
	ccmErrorsSinceReset = 0;
	setDisplay("check ok");
}

ObcCheck::~ObcCheck()
{

}

void ObcCheck::wake()
{
    runTask();
}

void ObcCheck::runTask()
{
	uint8_t rawByte = obc.ccm->getRawByte();
	uint8_t checkByte = obc.ccm->getCCMByte();
	uint8_t newErrors = checkByte & ~ccmErrorsSinceReset;
	if(newErrors)
	{
		DEBUG("CCM raw byte: 0x%02x new errors: 0x%02x (old errors: 0x%02x)\r\n", rawByte, newErrors, ccmErrorsSinceReset);
		if((rawByte == 0xff) && (checkByte != 0x00))
			setDisplay("check module failure");
		else if(newErrors & WasherFluid)
			setDisplay("washer fluid low");
		else if(newErrors & CoolantLevel)
			setDisplay("coolant low");
		else if(newErrors & LowBeam)
			setDisplay("low beam failure");
		else if(newErrors & LicensePlateLight)
			setDisplay("license light fail");
		else if(newErrors & TailLight)
			setDisplay("tail light failure");
		else if(newErrors & BrakeLight1)
			setDisplay("brake1 light failure");
		else if(newErrors & BrakeLight2)
			setDisplay("brake2 light failure");
		obc.ui->setActiveTask(this, 10);
		obc.ccmLight->on();
		obc.ui->callback.addCallback(this, &ObcCheck::errorTimeout, 10000);
	}
	ccmErrorsSinceReset |= newErrors;
}

void ObcCheck::buttonHandler(ObcUITaskFocus::type focus, uint32_t buttonMask)
{
	if(focus == ObcUITaskFocus::background)
	{
		if(buttonMask == BUTTON_CHECK_MASK)
			obc.ui->setActiveTask(this);
		return;
	}
	
	if(buttonMask == BUTTON_SET_MASK)
	{
		ccmErrorsSinceReset = 0;
		setDisplay("check ok");
	}
}

void ObcCheck::sleep()
{
    
}

void ObcCheck::errorTimeout()
{
	obc.ccmLight->off();
}

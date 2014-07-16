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
#include <algorithm>

using namespace ObcCCMBits;

ObcCheck::ObcCheck(OpenOBC& obc) : ObcUITask(obc)
{
	registerButton(ObcUITaskFocus::background, BUTTON_CHECK_MASK);
	setDisplay("check ...");
	isValid = false;
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
	updateDisplay();

	uint8_t rawByte = obc.ccm->getRawByte();
	static uint8_t lastRawByte;
	static uint16_t matchCount;

	if(rawByte == lastRawByte)
	{
		matchCount++;
		if(matchCount < 60) //2 seconds
		{
			return;
		}
		matchCount = 0;
	}
	else
	{
		matchCount = 0;
		lastRawByte = rawByte;
		return;
	}

	if(rawByte == 0xff) //ccm off or not connected
	{
		isValid = false;
		return;
	}

	if(rawByte & 1) //bit 0 is always 0
	{
		isValid = false;
		return;
	}

	isValid = true;

	uint8_t checkByte = obc.ccm->getCCMByte();

	if((checkByte & WasherFluid) && (std::find(warnings.begin(), warnings.end(), WasherFluid) == warnings.end()))
		newWarning(WasherFluid);
	if((checkByte & CoolantLevel) && (std::find(warnings.begin(), warnings.end(), CoolantLevel) == warnings.end()))
		newWarning(CoolantLevel);
	if((checkByte & LowBeam) && (std::find(warnings.begin(), warnings.end(), LowBeam) == warnings.end()))
		newWarning(LowBeam);
	if((checkByte & LicensePlateLight) && (std::find(warnings.begin(), warnings.end(), LicensePlateLight) == warnings.end()))
		newWarning(LicensePlateLight);
	if((checkByte & TailLight) && (std::find(warnings.begin(), warnings.end(), TailLight) == warnings.end()))
		newWarning(TailLight);
	if((checkByte & BrakeLight1) && (std::find(warnings.begin(), warnings.end(), BrakeLight1) == warnings.end()))
		newWarning(BrakeLight1);
	if((checkByte & BrakeLight2) && (std::find(warnings.begin(), warnings.end(), BrakeLight2) == warnings.end()))
		newWarning(BrakeLight2);
}

void ObcCheck::buttonHandler(ObcUITaskFocus::type focus, uint32_t buttonMask)
{
	if(focus == ObcUITaskFocus::background)
	{
		if(buttonMask == BUTTON_CHECK_MASK)
			obc.ui->setActiveTask(this);
		return;
	}

	if(buttonMask == BUTTON_CHECK_MASK)
	{
		if(!warnings.empty())
		{
			warnings.push_front(warnings.back());
			warnings.pop_back();
		}
	}
	
	if(buttonMask == BUTTON_SET_MASK)
	{
		while(!warnings.empty())
			warnings.pop_back();

		obc.ui->callback.deleteCallback(this, &ObcCheck::errorTimeout);
		obc.ccmLight->off();
	}
}

void ObcCheck::sleep()
{

}

void ObcCheck::errorTimeout()
{
	obc.ccmLight->off();
}

void ObcCheck::updateDisplay()
{
	if(!isActive())
	{
		obc.lcd->setSymbol(ObcLcdSymbols::Plus, false);
		return;
	}
	if(!isValid)
	{
		setDisplay("CCM not found");
		obc.lcd->setSymbol(ObcLcdSymbols::Plus, false);
		return;
	}
	if(warnings.empty())
	{
		setDisplay("check ok");
		obc.lcd->setSymbol(ObcLcdSymbols::Plus, false);
		return;
	}

	ObcCCMBits::bits currentWarning = warnings.back();

	if(currentWarning == WasherFluid)
		setDisplay("washer fluid low");
	else if(currentWarning == CoolantLevel)
		setDisplay("coolant low");
	else if(currentWarning == LowBeam)
		setDisplay("low beam failure");
	else if(currentWarning == LicensePlateLight)
		setDisplay("license light fail");
	else if(currentWarning == TailLight)
		setDisplay("tail light failure");
	else if(currentWarning == BrakeLight1)
		setDisplay("brake1 light failure");
	else if(currentWarning == BrakeLight2)
		setDisplay("brake2 light failure");

	if(warnings.size() > 1)
		obc.lcd->setSymbol(ObcLcdSymbols::Plus, true);
	else
		obc.lcd->setSymbol(ObcLcdSymbols::Plus, false);
}

void ObcCheck::newWarning(ObcCCMBits::bits warning)
{
	warnings.push_back(warning);
	obc.ui->setActiveTask(this, 10);
	obc.ccmLight->on();
	obc.ui->callback.deleteCallback(this, &ObcCheck::errorTimeout);
	obc.ui->callback.addCallback(this, &ObcCheck::errorTimeout, 10000);
	DEBUG("CCM raw byte: 0x%02x new warning: 0x%02x (%i warnings total)\r\n", obc.ccm->getRawByte(), warning, warnings.size());
}

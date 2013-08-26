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

#include "ObcCode.h"
#include <ObcUI.h>

using namespace ObcCodeState;

ObcCode::ObcCode(OpenOBC& obc) : ObcUITask(obc)
{
	registerButton(ObcUITaskFocus::background, BUTTON_CODE_MASK);
	setDisplayRefreshPeriod(0.100);
	setDisplay("ObcCode");
	state = CodeInactive;
}

ObcCode::~ObcCode()
{

}

void ObcCode::runTask()
{
	if(isActive())
	{
// 		DEBUG("running, ACTIVE\r\n");
	}
	else
	{
// 		DEBUG("running, INACTIVE\r\n");
	}
}

void ObcCode::wake()
{
	DEBUG("wake\r\n");
    if((state == CodeArmed) || (state == CodePrompt))
	{
		state = CodePrompt;
		codeSet = 0;
		setDisplay("input code: %04i", codeSet);
		obc.ui->setActiveTask(this);
	}
}

void ObcCode::sleep()
{
	DEBUG("sleep\r\n");
	if((state == CodeArmed) || (state == CodePrompt))
	{
		state = CodeArmed;
		obc.ews->on();
	}
}

void ObcCode::buttonHandler(ObcUITaskFocus::type focus, uint32_t buttonMask)
{
// 	DEBUG("buttonHandler\r\n");
	if(focus == ObcUITaskFocus::background)
	{
		if(buttonMask == BUTTON_CODE_MASK)
			obc.ui->setActiveTask(this);
		return;
	}
	
	if(state == CodeSet)
	{
		if(buttonMask == BUTTON_SET_MASK)
		{
			code = codeSet;
			state = CodeActive;
			setDisplay("code active");
			obc.codeLed->on();
		}
		if(buttonMask == BUTTON_CODE_MASK)
		{
			state = CodeInactive;
			setDisplay("code inactive");
			obc.codeLed->off();
		}
	}
	
	else if(state == CodeActive)
	{
		if(buttonMask == BUTTON_SET_MASK)
		{
			state = CodeSet;
			codeSet = code;
			setDisplay("set code: %04i", codeSet);
		}
	}
	
	else if(state == CodeInactive)
	{
		if(buttonMask == BUTTON_SET_MASK)
		{
			state = CodeSet;
			codeSet = code;
			setDisplay("set code: %04i", codeSet);
		}
	}
	
	else if(state == CodePrompt)
	{
		if(buttonMask == BUTTON_SET_MASK)
		{
			if(code == codeSet)
			{
				state = CodeActive;
				setDisplay("code accepted");
				obc.ews->off();
			}
			else
			{
				setDisplay("code invalid: %04i", codeSet);
				codeSet = 0;
			}
		}
	}
	
	if((state == CodeSet) || (state == CodePrompt))
	{
		if(buttonMask == BUTTON_1000_MASK)
		{
			codeSet += 1000;
		}
		if(buttonMask == BUTTON_100_MASK)
		{
			codeSet += 100;
		}
		if(buttonMask == BUTTON_10_MASK)
		{
			codeSet += 10;
		}
		if(buttonMask == BUTTON_1_MASK)
		{
			codeSet += 1;
		}
		
		codeSet %= 10000;
		setDisplay("input code: %04i", codeSet);
	}
}

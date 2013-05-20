/*
 *  Copyright (c) 2012 <benemorius@gmail.com>
 * 
 *  Permission is hereby granted, free of charge, to any person
 *  obtaining a copy of this software and associated documentation
 *  files (the "Software"), to deal in the Software without
 *  restriction, including without limitation the rights to use,
 *  copy, modify, merge, publish, distribute, sublicense, and/or sell
 *  copies of the Software, and to permit persons to whom the
 *  Software is furnished to do so, subject to the following
 *  conditions:
 * 
 *  The above copyright notice and this permission notice shall be
 *  included in all copies or substantial portions of the Software.
 * 
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 *  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 *  OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 *  NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 *  HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 *  WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 *  FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 *  OTHER DEALINGS IN THE SOFTWARE.
 */

#include "Timer.h"

#include <lpc17xx_timer.h>
#include <core_cmFunc.h>
#include <debugpretty.h>

bool Timer::timerInitialized = false;
std::list<uint32_t> Timer::matchValues;


Timer::Timer() : interruptManager(NULL)
{
	initialize();
	start();
}
Timer::Timer(InterruptManager& interruptManager) : interruptManager(&interruptManager)
{
	interruptManager.attach(TIMER_MANAGED_INTERRUPT, this, &Timer::interruptHandler);

	NVIC_SetPriority(TIMER_INTERRUPT, ((0x01<<3)|0x01));
	NVIC_EnableIRQ(TIMER_INTERRUPT);

	initialize();
	start();
}

void Timer::initialize()
{
	callbackFunction = new FunctionPointer<void>;
	callbackActive = false;
	
	if(!timerInitialized)
	{
		timerInitialized = true;
		
		//configure timer for 25MHz, overflow at 2^32 / 25000000 = ~171.798 seconds
		TIM_TIMERCFG_Type TIM_ConfigStruct;
		TIM_ConfigStruct.PrescaleOption = TIM_PRESCALE_TICKVAL;
		TIM_ConfigStruct.PrescaleValue	= 1;
		TIM_Init(TIMER_PERIPHERAL, TIM_TIMER_MODE, &TIM_ConfigStruct);
		
		TIM_MATCHCFG_Type TIM_MatchConfigStruct;
		TIM_MatchConfigStruct.MatchChannel = 0;
		TIM_MatchConfigStruct.IntOnMatch   = TRUE;
		TIM_MatchConfigStruct.ResetOnMatch = TRUE;
		TIM_MatchConfigStruct.StopOnMatch  = FALSE;
		TIM_MatchConfigStruct.ExtMatchOutputType = TIM_EXTMATCH_NOTHING;
		TIM_MatchConfigStruct.MatchValue   = 0xffffffff;
		TIM_ConfigMatch(TIMER_PERIPHERAL,&TIM_MatchConfigStruct);
		
		TIM_ResetCounter(TIMER_PERIPHERAL);
		TIM_Cmd(TIMER_PERIPHERAL,ENABLE);
	}
}

Timer::~Timer()
{
	__disable_irq();
	if(interruptManager != NULL)
	{
		interruptManager->detach(TIMER_MANAGED_INTERRUPT, this, &Timer::interruptHandler);
		
		//disable the interrupt if nothing remains attached to it
		if(!interruptManager->isAttached(TIMER_MANAGED_INTERRUPT))
			NVIC_DisableIRQ(TIMER_INTERRUPT);
	}
	if(callbackActive)
		_removeMatch(matchValue);
	if(callbackFunction != NULL)
		delete callbackFunction;
	__enable_irq();
}

//insert timerMatchValue chronologically into list according to current timer position
void Timer::_insertMatch(uint32_t timerMatchValue)
{
	__disable_irq();
	std::list<uint32_t>::iterator position;
	if(timerMatchValue > TIMER_PERIPHERAL->TC)
	{
		position = matchValues.begin();
		while(position != matchValues.end() && timerMatchValue > *position)
			++position;
		if(position != matchValues.end())
			--position;
	}
	else
	{
		position = matchValues.end();
		--position;
		while(position != matchValues.begin() && timerMatchValue < *position)
			--position;
		if(position != matchValues.begin())
			++position;
	}
	matchValues.insert(position, timerMatchValue);
	_updateMatch();
	__enable_irq();
}

void Timer::_removeMatch(uint32_t timerMatchValue)
{
	__disable_irq();
	for(std::list<uint32_t>::iterator position = matchValues.begin(); position != matchValues.end(); ++position)
	{
		if(*position == timerMatchValue)
		{
			matchValues.erase(position);
			break;
		}
	}
	_updateMatch();
	__enable_irq();
}

void Timer::_updateMatch()
{
	if(matchValues.empty())
		_clearMatch();

	if(matchValues.size() == 1)
		_setMatch(*(matchValues.begin()));
	
	//FIXME
}

void Timer::_setMatch(uint32_t timerMatchValue)
{
	TIM_MATCHCFG_Type TIM_MatchConfigStruct;
	TIM_MatchConfigStruct.MatchChannel = 1;
	TIM_MatchConfigStruct.IntOnMatch =  TRUE;
	TIM_MatchConfigStruct.ResetOnMatch = FALSE;
	TIM_MatchConfigStruct.StopOnMatch = FALSE;
	TIM_MatchConfigStruct.ExtMatchOutputType = TIM_EXTMATCH_NOTHING;
	TIM_MatchConfigStruct.MatchValue = timerMatchValue;
	TIM_ConfigMatch(TIMER_PERIPHERAL, &TIM_MatchConfigStruct);
}

void Timer::_clearMatch()
{
	TIM_MATCHCFG_Type TIM_MatchConfigStruct;
	TIM_MatchConfigStruct.MatchChannel = 1;
	TIM_MatchConfigStruct.IntOnMatch =  FALSE;
	TIM_MatchConfigStruct.ResetOnMatch = FALSE;
	TIM_MatchConfigStruct.StopOnMatch = FALSE;
	TIM_MatchConfigStruct.ExtMatchOutputType = TIM_EXTMATCH_NOTHING;
	TIM_MatchConfigStruct.MatchValue = 0;
	TIM_ConfigMatch(TIMER_PERIPHERAL, &TIM_MatchConfigStruct);
}

void Timer::start()
{
	__disable_irq();
	overflows = 0;
	startCount = TIMER_PERIPHERAL->TC;
	if(callbackFunction->isValid())
	{
		if(callbackActive)
		{
			setCallbackDelay(callbackDelay);
		}
		else
		{
			matchValue = _computeMatchValue(callbackDelay);
			_insertMatch(matchValue);
			callbackActive = true;
		}
	}
	__enable_irq();
}

uint32_t Timer::read()
{
	uint32_t counter = TIMER_PERIPHERAL->TC;
	uint32_t ret;
	if(counter >= startCount)
	{
		ret = counter - startCount;
		overflowed = false;
	}
	else
	{
		ret = counter + (0xffffffff - startCount);
		overflowed = true;
	}
	
	return ret;
}

uint32_t Timer::read_ms()
{
	uint32_t ms = read() / 25000;
	
	if((overflows > 0) && (overflowed == true))
		return ms + (((uint32_t)0xffffffff / 25000) * (overflows - 1));
	else if((overflows > 0) && (overflowed == false))
		return ms + (((uint32_t)0xffffffff / 25000) * overflows);
	else
		return ms;
}

uint32_t Timer::read_us()
{
	uint32_t us = read() / 25;
	
	if((overflows > 0) && (overflowed == true))
		return us + (((uint32_t)0xffffffff / 25) * (overflows - 1));
	else if((overflows > 0) && (overflowed == false))
		return us + (((uint32_t)0xffffffff / 25) * overflows);
	else
		return us;
}

void Timer::interruptHandler(bool isLast)
{
// 	DEBUG("timer irq 0x%x\r\n", this);
	if(TIM_GetIntStatus(TIMER_PERIPHERAL, TIM_MR0_INT) == SET)
	{
// 		DEBUG("overflow 0x%x\r\n", this);
		if(isLast)
		{
			TIM_ClearIntPending(TIMER_PERIPHERAL, TIM_MR0_INT);
		}
		overflows++;
	}
	if(TIM_GetIntStatus(TIMER_PERIPHERAL, TIM_MR1_INT) == SET)
	{
		if(isLast)
		{
// 			DEBUG("clearing 0x%x\r\n", this);
			TIM_ClearIntPending(TIMER_PERIPHERAL, TIM_MR1_INT);
		}
		if(callbackActive && read_us() >= callbackDelay)
		{
			frozenTimerValue = TIMER_PERIPHERAL->TC;
// 			DEBUG("calling 0x%x\r\n", this);
			callbackActive = false;
			_removeMatch(matchValue);
			callbackFunction->call();
		}
		else
		{
// 			DEBUG("not calling 0x%x\r\n", this);
		}
	}
}

void Timer::setCallbackDelay(uint32_t microseconds)
{
	__disable_irq();
	callbackDelay = microseconds;

	if(callbackActive)
	{
		_removeMatch(matchValue);
		matchValue = _computeMatchValue(callbackDelay);
		_insertMatch(matchValue);
	}
	__enable_irq();
}

void Timer::deleteCallback()
{
	__disable_irq();
	callbackActive = false;
	_removeMatch(matchValue);
	callbackFunction->detachAll();
	__enable_irq();
}

uint32_t Timer::_computeMatchValue(uint32_t microseconds)
{
	uint64_t matchValue = (uint64_t)microseconds * 25 + (TIMER_PERIPHERAL->TC);
	matchValue %= 0x100000000;
	return (matchValue & 0xffffffff);
}

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

bool Timer::timerInitialized = false;

Timer::Timer() : interruptManager(0)
{
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
	
	start();
}
Timer::Timer(InterruptManager& interruptManager) : interruptManager(&interruptManager)
{
	interruptManager.attach(TIMER_MANAGED_INTERRUPT, this, &Timer::overflowHandler);

	NVIC_SetPriority(TIMER_INTERRUPT, ((0x01<<3)|0x01));
	NVIC_EnableIRQ(TIMER_INTERRUPT);

	Timer();
}

Timer::~Timer()
{
	if(interruptManager)
	{
		interruptManager->detach(TIMER_MANAGED_INTERRUPT, this, &Timer::overflowHandler);

		//disable the interrupt if nothing remains attached to it
		if(!interruptManager->isAttached(TIMER_MANAGED_INTERRUPT))
			NVIC_DisableIRQ(TIMER_INTERRUPT);
	}
}

void Timer::start()
{
	overflows = 0;
	startCount = TIMER_PERIPHERAL->TC;
}

uint32_t Timer::read()
{
	return TIMER_PERIPHERAL->TC - startCount;
}

uint32_t Timer::read_ms()
{
	int32_t ms = (int32_t)(TIMER_PERIPHERAL->TC / 25000) - (startCount / 25000);
	return ms + ((0xffffffff / 25000) * overflows);
}

uint32_t Timer::read_us()
{
	int32_t us = (int32_t)(TIMER_PERIPHERAL->TC / 25) - (startCount / 25);
	return us + ((0xffffffff / 25) * overflows);
}

void Timer::overflowHandler()
{
	if(TIM_GetIntStatus(TIMER_PERIPHERAL, TIM_MR0_INT) == SET)
	{
		TIM_ClearIntPending(TIMER_PERIPHERAL, TIM_MR0_INT);
		overflows++;
	}
}

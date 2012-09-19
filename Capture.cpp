/*
    Copyright (c) 2012 <benemorius@gmail.com>

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

#include "Capture.h"
#include <lpc17xx_pinsel.h>
#include <lpc17xx_timer.h>

Capture::Capture(uint8_t speedPort, uint8_t speedPin, uint8_t fuelConsPort, uint8_t fuelConsPin, InterruptManager* interruptManager) : speedPort(speedPort), speedPin(speedPin), fuelConsPort(fuelConsPort), fuelConsPin(fuelConsPin)
{
	onTime = 0;

	
	PINSEL_CFG_Type PinCfg;
	PinCfg.OpenDrain = PINSEL_PINMODE_NORMAL;
	PinCfg.Pinmode = PINSEL_PINMODE_PULLUP; //TODO make this configurable

	PinCfg.Funcnum = 3; //FIXME
	this->peripheral = LPC_TIM2; //FIXME
	this->irqn = TIMER2_IRQn; //FIXME
	IRQHandler_Type irq = IRQ_TIMER2; //FIXME

// 	this->channel = 0; //FIXME
	
// 	PinCfg.Portnum = speedPort;
// 	PinCfg.Pinnum = speedPin;
// 	PINSEL_ConfigPin(&PinCfg);
	PinCfg.Portnum = fuelConsPort;
	PinCfg.Pinnum = fuelConsPin;
	PINSEL_ConfigPin(&PinCfg);

	
	TIM_TIMERCFG_Type TIM_ConfigStruct;

	// Initialize timer, prescale count time of 1000000uS = 1S
// 	TIM_ConfigStruct.PrescaleOption = TIM_PRESCALE_USVAL;
	TIM_ConfigStruct.PrescaleOption = TIM_PRESCALE_TICKVAL;
	TIM_ConfigStruct.PrescaleValue	= 1;
	TIM_Init(this->peripheral, TIM_TIMER_MODE, &TIM_ConfigStruct);

	TIM_CAPTURECFG_Type TIM_CaptureConfigStruct;
// 	TIM_CaptureConfigStruct.CaptureChannel = 0;
// 	TIM_CaptureConfigStruct.RisingEdge = ENABLE;
// 	TIM_CaptureConfigStruct.FallingEdge = DISABLE;
// 	TIM_CaptureConfigStruct.IntOnCaption = ENABLE;
// 	TIM_ConfigCapture(this->peripheral, &TIM_CaptureConfigStruct);
	TIM_CaptureConfigStruct.CaptureChannel = 1;
	TIM_CaptureConfigStruct.RisingEdge = ENABLE;
	TIM_CaptureConfigStruct.FallingEdge = DISABLE;
	TIM_CaptureConfigStruct.IntOnCaption = ENABLE;
	TIM_ConfigCapture(this->peripheral, &TIM_CaptureConfigStruct);

	TIM_ResetCounter(this->peripheral);

	interruptManager->attach(irq, this, &Capture::interruptHandler);
	
	/* preemption = 1, sub-priority = 1 */
	NVIC_SetPriority(this->irqn, ((0x01<<3)|0x01));
	/* Enable interrupt for timer 0 */
	NVIC_EnableIRQ(this->irqn);
	// To start timer 0
	TIM_Cmd(this->peripheral,ENABLE);
	
}

void Capture::interruptHandler()
{
	static bool ch0High;

	static bool ch1High;
	static uint32_t ch1RisingEdge;
	static uint32_t ch1FallingEdge;
	static uint32_t ch1LastFallingEdge;
	
	if(TIM_GetIntCaptureStatus(this->peripheral,TIM_MR0_INT)) //FIXME
	{
		TIM_ClearIntCapturePending(this->peripheral,TIM_MR0_INT); //FIXME
		if(!ch0High)
		{
			ch0High = true;
			risingEdge = TIM_GetCaptureValue(this->peripheral,TIM_COUNTER_INCAP0); //FIXME
			TIM_CAPTURECFG_Type TIM_CaptureConfigStruct;
			TIM_CaptureConfigStruct.CaptureChannel = 0;
			TIM_CaptureConfigStruct.RisingEdge = DISABLE;
			TIM_CaptureConfigStruct.FallingEdge = ENABLE;
			TIM_CaptureConfigStruct.IntOnCaption = ENABLE;
			TIM_ConfigCapture(this->peripheral, &TIM_CaptureConfigStruct);
		}
		else
		{
			if(lastFallingEdge != 0)
				TIM_ResetCounter(this->peripheral);
			ch0High = false;
			fallingEdge = TIM_GetCaptureValue(this->peripheral,TIM_COUNTER_INCAP0); //FIXME
			TIM_CAPTURECFG_Type TIM_CaptureConfigStruct;
			TIM_CaptureConfigStruct.CaptureChannel = 0;
			TIM_CaptureConfigStruct.RisingEdge = ENABLE;
			TIM_CaptureConfigStruct.FallingEdge = DISABLE;
			TIM_CaptureConfigStruct.IntOnCaption = ENABLE;
			TIM_ConfigCapture(this->peripheral, &TIM_CaptureConfigStruct);
			if(lastFallingEdge != 0)
			{
				period = (float)(fallingEdge - lastFallingEdge) / CLKPWR_GetPCLK(CLKPWR_PCLKSEL_TIMER2); //FIXME
				dutyCycle = (float)(fallingEdge - risingEdge) / (fallingEdge - lastFallingEdge);
				lastFallingEdge = 0;
			}
			else
				lastFallingEdge = fallingEdge;
		}
		
// 		callback.call();
	}

	if(TIM_GetIntCaptureStatus(this->peripheral,TIM_MR1_INT)) //FIXME
	{
		TIM_ClearIntCapturePending(this->peripheral,TIM_MR1_INT); //FIXME
		if(!ch1High)
		{
			ch1High = true;
			ch1RisingEdge = TIM_GetCaptureValue(this->peripheral,TIM_COUNTER_INCAP1); //FIXME
			TIM_CAPTURECFG_Type TIM_CaptureConfigStruct;
			TIM_CaptureConfigStruct.CaptureChannel = 1;
			TIM_CaptureConfigStruct.RisingEdge = DISABLE;
			TIM_CaptureConfigStruct.FallingEdge = ENABLE;
			TIM_CaptureConfigStruct.IntOnCaption = ENABLE;
			TIM_ConfigCapture(this->peripheral, &TIM_CaptureConfigStruct);
		}
		else
		{
			TIM_ResetCounter(this->peripheral);
			
			ch1High = false;
			ch1FallingEdge = TIM_GetCaptureValue(this->peripheral,TIM_COUNTER_INCAP1); //FIXME
			TIM_CAPTURECFG_Type TIM_CaptureConfigStruct;
			TIM_CaptureConfigStruct.CaptureChannel = 1;
			TIM_CaptureConfigStruct.RisingEdge = ENABLE;
			TIM_CaptureConfigStruct.FallingEdge = DISABLE;
			TIM_CaptureConfigStruct.IntOnCaption = ENABLE;
			TIM_ConfigCapture(this->peripheral, &TIM_CaptureConfigStruct);

			uint32_t onClocks = ch1FallingEdge - ch1RisingEdge;
			onTime = (float)onClocks / CLKPWR_GetPCLK(CLKPWR_PCLKSEL_TIMER2);
// 			if(onTime > 10000)
// 				onTime = 0;
			
		}
		
		callback.call();
	}
}


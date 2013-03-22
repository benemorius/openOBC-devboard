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

#include "PWM.h"

#include <lpc17xx_pinsel.h>
#include <lpc17xx_clkpwr.h>

#include <stdio.h>


//FIXME can't instantiate more than one PWM at a time
PWM::PWM(uint8_t port, uint8_t pin, float dutyCycle, float frequency)
{
	this->port = port;
	this->pin = pin;
	this->dutyCycle = dutyCycle;
	this->frequency = 100000;

	PINSEL_CFG_Type PinCfg;
	if(port == 1 && pin == 18)
	{
		PinCfg.Funcnum = 2;
		this->channel = 1;
		this->peripheral = LPC_PWM1;
	}
	else if(port == 1 && pin == 20)
	{
		PinCfg.Funcnum = 2;
		this->channel = 2;
		this->peripheral = LPC_PWM1;
	}
	else if(port == 1 && pin == 21)
	{
		PinCfg.Funcnum = 2;
		this->channel = 3;
		this->peripheral = LPC_PWM1;
	}
	else if(port == 1 && pin == 23)
	{
		PinCfg.Funcnum = 2;
		this->channel = 4;
		this->peripheral = LPC_PWM1;
	}
	else if(port == 1 && pin == 24)
	{
		PinCfg.Funcnum = 2;
		this->channel = 5;
		this->peripheral = LPC_PWM1;
	}
	else if(port == 1 && pin == 26)
	{
		PinCfg.Funcnum = 2;
		this->channel = 6;
		this->peripheral = LPC_PWM1;
	}
	else if(port == 2 && pin == 0)
	{
		PinCfg.Funcnum = 1;
		this->channel = 1;
		this->peripheral = LPC_PWM1;
	}
	else if(port == 2 && pin == 1)
	{
		PinCfg.Funcnum = 1;
		this->channel = 2;
		this->peripheral = LPC_PWM1;
	}
	else if(port == 2 && pin == 2)
	{
		PinCfg.Funcnum = 1;
		this->channel = 3;
		this->peripheral = LPC_PWM1;
	}
	else if(port == 2 && pin == 3)
	{
		PinCfg.Funcnum = 1;
		this->channel = 4;
		this->peripheral = LPC_PWM1;
	}
	else if(port == 2 && pin == 4)
	{
		PinCfg.Funcnum = 1;
		this->channel = 5;
		this->peripheral = LPC_PWM1;
	}
	else if(port == 2 && pin == 5)
	{
		PinCfg.Funcnum = 1;
		this->channel = 6;
		this->peripheral = LPC_PWM1;
	}
	else if(port == 3 && pin == 25)
	{
		PinCfg.Funcnum = 3;
		this->channel = 2;
		this->peripheral = LPC_PWM1;
	}
	else if(port == 2 && pin == 26)
	{
		PinCfg.Funcnum = 3;
		this->channel = 3;
		this->peripheral = LPC_PWM1;
	}
	else
	{
		//invalid port/pin specified
// 		check_failed((uint8_t *)__FILE__, __LINE__);
		while(1);
	}

	PinCfg.Portnum = port;
	PinCfg.Pinnum = pin;
	PinCfg.OpenDrain = PINSEL_PINMODE_NORMAL;
	PinCfg.Pinmode = PINSEL_PINMODE_TRISTATE;
	PINSEL_ConfigPin(&PinCfg);

	
	/* PWM block section -------------------------------------------- */
	/* Initialize PWM peripheral, timer mode
	 * PWM prescale value = 1 (absolute value - tick value) */
	PWM_TIMERCFG_Type PWMCfgDat;
	PWMCfgDat.PrescaleOption = PWM_TIMER_PRESCALE_TICKVAL;
	PWMCfgDat.PrescaleValue = 1;
	PWM_Init(this->peripheral, PWM_MODE_TIMER, (void*)&PWMCfgDat);

	uint32_t pwmclk = CLKPWR_GetPCLK(CLKPWR_PCLKSEL_PWM1);

	/* Set match value for PWM match channel 0 and reset on match to set the period for all channels */
	PWM_MatchUpdate(this->peripheral, 0, pwmclk / this->frequency, PWM_MATCH_UPDATE_NOW);
	PWM_MATCHCFG_Type PWMMatchCfgDat;
	PWMMatchCfgDat.IntOnMatch = DISABLE;
	PWMMatchCfgDat.MatchChannel = 0;
	PWMMatchCfgDat.ResetOnMatch = ENABLE;
	PWMMatchCfgDat.StopOnMatch = DISABLE;
	PWM_ConfigMatch(this->peripheral, &PWMMatchCfgDat);


	/* Configure each PWM channel: --------------------------------------------- */
	/* - Single edge
	 * - PWM Duty on each PWM channel determined by
	 * the match on channel 0 to the match of that match channel.
	 * Example: PWM Duty on PWM channel 1 determined by
	 * the match on channel 0 to the match of match channel 1.
	 */
	PWM_ChannelConfig(this->peripheral, this->channel, PWM_CHANNEL_SINGLE_EDGE);


	/* Set up match value */
	PWM_MatchUpdate(this->peripheral, this->channel, this->dutyCycle * (pwmclk / this->frequency), PWM_MATCH_UPDATE_NOW);
	PWMMatchCfgDat.IntOnMatch = DISABLE;
	PWMMatchCfgDat.MatchChannel = this->channel;
	PWMMatchCfgDat.ResetOnMatch = DISABLE;
	PWMMatchCfgDat.StopOnMatch = DISABLE;
	PWM_ConfigMatch(this->peripheral, &PWMMatchCfgDat);
	
	/* Enable PWM Channel Output */
	PWM_ChannelCmd(this->peripheral, this->channel, ENABLE);
	
	/* Reset and Start counter */
	PWM_ResetCounter(this->peripheral);
	PWM_CounterCmd(this->peripheral, ENABLE);
	
	/* Start PWM now */
	PWM_Cmd(this->peripheral, ENABLE);
}

float PWM::getDutyCycle()
{
	return this->dutyCycle;
}

void PWM::setDutyCycle(float dutyCycle)
{
	this->dutyCycle = dutyCycle;
	uint32_t pwmclk = CLKPWR_GetPCLK(CLKPWR_PCLKSEL_PWM1);
	PWM_MatchUpdate(this->peripheral, this->channel, this->dutyCycle * (pwmclk / this->frequency), PWM_MATCH_UPDATE_NEXT_RST);
}

float PWM::getFrequency()
{
	return this->frequency;
}

void PWM::setFrequency(float frequency)
{
	if(frequency > 1)
		frequency = 1;
	if(frequency < 0)
		frequency = 0;
	this->frequency = frequency;
	uint32_t pwmclk = CLKPWR_GetPCLK(CLKPWR_PCLKSEL_PWM1);
	PWM_MatchUpdate(this->peripheral, 0, pwmclk / this->frequency, PWM_MATCH_UPDATE_NOW);

	PWM_MATCHCFG_Type PWMMatchCfgDat;
	PWMMatchCfgDat.IntOnMatch = DISABLE;
	PWMMatchCfgDat.MatchChannel = 0;
	PWMMatchCfgDat.ResetOnMatch = ENABLE;
	PWMMatchCfgDat.StopOnMatch = DISABLE;
	PWM_ConfigMatch(this->peripheral, &PWMMatchCfgDat);
	
// 	PWM_ChannelCmd(this->peripheral, this->channel, ENABLE);
// 	
// 	PWM_ResetCounter(this->peripheral);
// 	PWM_CounterCmd(this->peripheral, ENABLE);
// 
// 	PWM_Cmd(this->peripheral, ENABLE);
	
}

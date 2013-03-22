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

#ifndef PWM_H
#define PWM_H

#include <lpc_types.h>
#include <lpc17xx_pwm.h>


class PWM
{
public:
	PWM(uint8_t port, uint8_t pin, float dutyCycle = .5, float frequency = 80000);
	void setDutyCycle(float dutyCycle);
	float getDutyCycle();
	void setFrequency(float frequency); //on the LPC17xx this will set the frequency of all 6 PWM channels
	float getFrequency();

private:
	uint8_t port;
	uint8_t pin;
	uint8_t channel;
	LPC_PWM_TypeDef* peripheral;
	float dutyCycle;
	float frequency;
};

#endif // PWM_H

/*
    Copyright (c) 2014 <benemorius@gmail.com>

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

#include "OilPressureSensor.h"

OilPressureSensor::OilPressureSensor(AnalogIn& analogIn, float calibrationScale) : analogIn(analogIn), calibrationScale(calibrationScale)
{

}

float OilPressureSensor::getPsi()
{
	return getPsiFromVoltage(analogIn.read()) * calibrationScale;
}

float OilPressureSensor::getBar()
{
	return getPsi() / 14.504;
}

float OilPressureSensor::getPsiFromVoltage(float voltage)
{
	if(voltage < 0.04)
		return -1;
	if(voltage > 0.5)
		return 999;

	float currentPsi = (float)pressureLookupTable[(unsigned int)(voltage * 100)] / 10;
	return currentPsi;
}

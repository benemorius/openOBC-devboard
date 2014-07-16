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

#ifndef OILPRESSURESENSOR_H
#define OILPRESSURESENSOR_H

#include <AnalogIn.h>

class OilPressureSensor
{
public:
	OilPressureSensor(AnalogIn& analogIn, float calibrationScale = 1.0f);

	float getPsi();
	float getBar();

	float getCalibrationScale() const {return calibrationScale;}
	float setCalibrationScale(float calibrationScale) {this->calibrationScale = calibrationScale;}

private:
	AnalogIn& analogIn;
	float calibrationScale;

	float getPsiFromVoltage(float voltage);
};

//for VDO 360-007, 80 psi 10-180 ohm sensor
//starts at 0mV; increments by 10mV
const uint16_t pressureLookupTable[] = {
	0, //0 psi == 0mV
	0, //0 psi == 10mV
	0,
	0,
	11, //1.1 psi == 40mV
	25, //2.5 psi == 50mV
	39,
	53,
	67,
	81,
	95,
	110,
	124,
	139,
	154,
	169,
	184,
	199,
	215,
	230,
	256,
	261,
	278,
	294,
	310,
	327,
	343,
	360,
	377,
	394,
	411,
	428,
	446,
	464,
	482,
	500,
	519,
	537,
	556,
	575,
	594,
	613,
	633,
	653,
	673,
	693,
	713,
	734,
	755,
	776,
	797
};

#endif // OILPRESSURESENSOR_H

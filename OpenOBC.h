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

#ifndef OPENOBC_H
#define OPENOBC_H

#include "FunctionPointer.h"
#include "Debug.h"
#include "RTC.h"
#include "PWM.h"
#include "SPI.h"
#include "ObcLcd.h"
#include "InterruptManager.h"
#include "Input.h"
#include "ObcKeypad.h"
#include "CheckControlModule.h"
#include "FuelLevel.h"
#include "SDFS.h"
#include "ConfigFile.h"
#include <AnalogIn.h>
#include "SpeedInput.h"
#include "E36ZKE4.h"
#include "E36SRS.h"
#include "E36IHKR.h"
#include "E36Kombi.h"
#include "E36MK4.h"
#include "FuelConsumption.h"

typedef enum
{
	DISPLAY_CONSUM1,
	DISPLAY_CONSUM2,
	DISPLAY_CONSUM3,
	DISPLAY_TEMP,
	DISPLAY_SPEED,
	DISPLAY_VOLTAGE,
	DISPLAY_OPENOBC,
	DISPLAY_CHECK,
	DISPLAY_FREEMEM,
	DISPLAY_FUEL_LEVEL,
	DISPLAY_OUTPUTS
} DisplayMode_Type;

class OpenOBC : public InterruptManagerOwner
{
	
public:
	OpenOBC();
	void mainloop();
	void buttonConsum();
	void buttonRange();
	void buttonTemp();
	void buttonSpeed();
	void buttonKMMLS();
	void button1();
	void buttonCheck();
	void buttonSet();
	void buttonMemo();
	void buttonDist();
	void button1000();
	void button100();
	void buttonClock();
	void buttonDate();
	void buttonTimer();
	
	SPI* spi1;
	IO* lcdLight;
	IO* clockLight;
	IO* keypadLight;
	Input* run;
	ObcKeypad* keypad;
	Input* sdcardDetect;
	FuelLevel* fuelLevel;
	Input* stalkButton;
	SDFS* sd;
	ConfigFile* config;
	DS2* diag;
	E36ZKE4* zke;
	E36SRS* srs;
	E36IHKR* ihkr;
	E36Kombi* kombi;
	E36MK4* mk4;
	

private:
	void sleep();
	void wake();
	void printDS2Packet();
	
	Debug* debug;
	RTC* rtc;
	Uart* kline;
	Uart* lline;
	PWM* lcdBacklight;
	PWM* clockBacklight;
	PWM* keypadBacklight;
	PWM* lcdBiasClock;
	ObcLcd* lcd;
	CheckControlModule* ccm;
	IO* lcdReset;
	IO* out0Cs;
	IO* out1Cs;
	DisplayMode_Type displayMode;
	AnalogIn* batteryVoltage;
	AnalogIn* temperature;
	SpeedInput* speed;
	FuelConsumption* fuelCons;
	bool useMetricSystem;
	
};



#endif //OPENOBC_H

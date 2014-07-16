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

#include "ObcLcd.h"

#include <stdarg.h>
#include <cstdio>
#include "delay.h"
#include <string.h>

ObcLcd::ObcLcd(SPI& spi, IO& cs, IO& refresh, IO& unk0, IO& unk1, uint32_t spiClockrateHz) : spi(spi), cs(cs), refresh(refresh), unk0(unk0), unk1(unk1), spiClockrate(spiClockrateHz)
{
	this->cs = true;
	this->refresh = false;
	this->unk0 = true;
	this->unk1 = false;
	*lcdBuffer = '\0';
	*clockBuffer = '\0';
	this->colonEnabled = false;
	this->dotsEnabled = false;
}

void ObcLcd::printf(char* format, ...)
{
	va_list args;
	va_start(args, format);
	vsnprintf(lcdBuffer, LCD_MAX_CHARACTERS+1, format, args);
	va_end(args);
	update();
}

void ObcLcd::printfClock(char* format, ...)
{
	va_list args;
	va_start(args, format);
	vsnprintf(clockBuffer, CLOCK_MAX_CHARACTERS+1, format, args);
	va_end(args);
	update();
}

void ObcLcd::clear()
{
	printf("");
}

void ObcLcd::clearClock()
{
	printfClock("");
}

void ObcLcd::update()
{
	spi.setClockRate(spiClockrate);

	testSymbols();

	cs = false;

	//center the lcd text within LCD_MAX_CHARACTERS characters
	uint32_t length = (LCD_MAX_CHARACTERS - strlen(lcdBuffer));
	uint32_t spaces = length / 2;
	if(length % 2) //if an odd length makes it uncenterable, shifting to the right looks better
		++spaces;
	for(uint8_t i = spaces; i; i--)
		spi.readWrite(' ');

	//send the lcd buffer; fill to LCD_MAX_CHARACTERS characters with spaces
	uint8_t i;
	for(i = 0; i < (LCD_MAX_CHARACTERS - spaces); i++)
	{
		if(lcdBuffer[i] == '\0')
			break;
		spi.readWrite((uint8_t)lcdBuffer[i]);
	}
	while(i++ < (LCD_MAX_CHARACTERS - spaces))
		spi.readWrite(' ');

	//send the clock buffer; fill to CLOCK_MAX_CHARACTERS characters with spaces
	for(i = 0; i < CLOCK_MAX_CHARACTERS; i++)
	{
		if(clockBuffer[i] == '\0')
			break;
		spi.readWrite((uint8_t)clockBuffer[i]);
	}
	while(i++ < CLOCK_MAX_CHARACTERS)
		spi.readWrite(' ');

	//I forget what or why this is
	spi.readWrite(0x08); //must be 0x08 to enable extra lcd symbols; 0x00 will disable them
	spi.readWrite(' ');
	spi.readWrite(' ');
	spi.readWrite(' ');

	refresh = true;
	delay(1);
	refresh = false;
	cs = true;
}

void ObcLcd::testSymbols()
{
	//                          am          dots        pm          bottom/+    right       top         memo        left
	const char   all[] = {0x07, 0x04, 0x06, 0x0c, 0x05, 0x04, 0x04, 0x0c, 0x03, 0x04, 0x02, 0x08, 0x01, 0x04, 0x00, 0x04, 0x0f, 0x00, 0x0e, 0x11, 0x0d, 0x13, 0x0c, 0x15, 0x0b, 0x19, 0x0a, 0x11, 0x09, 0x00, 0x08, 0x1f};
	const char  none[] = {0x07, 0x00, 0x06, 0x00, 0x05, 0x00, 0x04, 0x00, 0x03, 0x00, 0x02, 0x00, 0x01, 0x00, 0x00, 0x00, 0x0f, 0x00, 0x0e, 0x11, 0x0d, 0x13, 0x0c, 0x15, 0x0b, 0x19, 0x0a, 0x11, 0x09, 0x00, 0x08, 0x1f};
	const char  some[] = {0x07, 0x00, 0x06, 0x00, 0x05, 0x00, 0x04, 0x08, 0x03, 0x00, 0x02, 0x08, 0x01, 0x04, 0x00, 0x00, 0x0f, 0x00, 0x0e, 0x11, 0x0d, 0x13, 0x0c, 0x15, 0x0b, 0x19, 0x0a, 0x11, 0x09, 0x00, 0x08, 0x1f};

	const char colon[] = {0x07, 0x00, 0x06, 0x00, 0x05, 0x00, 0x04, 0x08, 0x03, 0x00, 0x02, 0x08, 0x01, 0x00, 0x00, 0x00, 0x0f, 0x00, 0x0e, 0x11, 0x0d, 0x13, 0x0c, 0x15, 0x0b, 0x19, 0x0a, 0x11, 0x09, 0x00, 0x08, 0x1f};
	const char  dots[] = {0x07, 0x00, 0x06, 0x0c, 0x05, 0x00, 0x04, 0x00, 0x03, 0x00, 0x02, 0x00, 0x01, 0x00, 0x00, 0x00, 0x0f, 0x00, 0x0e, 0x11, 0x0d, 0x13, 0x0c, 0x15, 0x0b, 0x19, 0x0a, 0x11, 0x09, 0x00, 0x08, 0x1f};
	int numStuff = 32;

	const char* stuff;
	if(colonEnabled)
		stuff = colon;
	else if(dotsEnabled)
		stuff = dots;
	else
		stuff = none;

	cs.off();
	for(int i = 0; i < numStuff; i += 2)
	{
		spi.readWrite(stuff[i]);
		spi.readWrite(stuff[i+1]);
		unk1.on();
		unk1.off();
	}
	cs.on();
	delay(1);
}


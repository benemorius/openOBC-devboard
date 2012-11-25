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

ObcLcd::ObcLcd(SPI& spi, IO& cs, IO& refresh, IO& unk0, IO& unk1) : spi(spi), cs(cs), refresh(refresh), unk0(unk0), unk1(unk1)
{
	this->cs = true;
	this->refresh = false;
	this->unk0 = true;
	this->unk1 = false;
	*lcdBuffer = '\0';
	*clockBuffer = '\0';
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

void ObcLcd::update()
{
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
	spi.readWrite(0x00);
	spi.readWrite(' ');
	spi.readWrite(' ');
	spi.readWrite(' ');

	refresh = true;
	delay(1);
	refresh = false;
	cs = true;
}



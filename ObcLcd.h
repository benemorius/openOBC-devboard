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

#ifndef OBCLCD_H
#define OBCLCD_H
#include "SPI.h"
#include "IO.h"

#define LCD_MAX_CHARACTERS (20)
#define CLOCK_MAX_CHARACTERS (4)

class ObcLcd
{
public:
	ObcLcd(SPI& spi, IO& cs, IO& refresh, IO& unk0, IO& unk1, uint32_t spiClockrateHz = 1000000);

	void printf(char* format, ...);
	void printfClock(char* format, ...);
	void clear();
	void clearClock();
	 

private:
	void update();

	SPI& spi;
	IO& cs;
	IO& refresh;
	IO& unk0;
	IO& unk1;
	uint32_t spiClockrate;
	
	char lcdBuffer[LCD_MAX_CHARACTERS+1];
	char clockBuffer[CLOCK_MAX_CHARACTERS+1];
};

#endif // OBCLCD_H

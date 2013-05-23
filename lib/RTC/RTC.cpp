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

#include "RTC.h"

#include "lpc17xx_rtc.h"

RTC::RTC()
{
	RTC_Init(LPC_RTC);
	RTC_ResetClockTickCounter(LPC_RTC);
	RTC_Cmd(LPC_RTC, ENABLE);
	
	
	/* Setting Timer calibration
	 * Calibration value =  5s;
	 * Direction = Forward calibration
	 * So after each 5s, calibration logic can periodically adjust the time counter by
	 * incrementing the counter by 2 instead of 1
	 */
	RTC_CalibConfig(LPC_RTC, 9000, RTC_CALIB_DIR_FORWARD);
	//leave calibration disabled
	RTC_CalibCounterCmd(LPC_RTC, DISABLE);
}

//TODO utc, timezone, dst
void RTC::getTime(RTC_TIME_Type* time)
{
	RTC_GetFullTime(LPC_RTC, time);
}

void RTC::setTime(RTC_TIME_Type* time)
{
	RTC_SetFullTime(LPC_RTC, time);
}

uint8_t RTC::getSecond()
{
	RTC_GetFullTime(LPC_RTC, &time);
	return time.SEC;
}

uint8_t RTC::getMinute()
{
	RTC_GetFullTime(LPC_RTC, &time);
	return time.MIN;
}

uint8_t RTC::getHour()
{
	RTC_GetFullTime(LPC_RTC, &time);
	return time.HOUR;
}

uint8_t RTC::getDay()
{
	RTC_GetFullTime(LPC_RTC, &time);
	return time.DOM;
}

uint8_t RTC::getMonth()
{
	RTC_GetFullTime(LPC_RTC, &time);
	return time.MONTH;
}

uint16_t RTC::getYear()
{
	RTC_GetFullTime(LPC_RTC, &time);
	return time.YEAR;
}

void RTC::setSecond(uint8_t second)
{
	second %= 60;
	RTC_SetTime(LPC_RTC, RTC_TIMETYPE_SECOND, second);
}

void RTC::setMinute(uint8_t minute)
{
	minute %= 60;
	RTC_SetTime(LPC_RTC, RTC_TIMETYPE_SECOND, 0);
	RTC_SetTime(LPC_RTC, RTC_TIMETYPE_MINUTE, minute);
}

void RTC::setHour(uint8_t hour)
{
	hour %= 24;
	RTC_SetTime(LPC_RTC, RTC_TIMETYPE_HOUR, hour);
}

void RTC::setDay(uint8_t day)
{
	day %= 32;
	if(day == 0)
		day = 1;
	RTC_SetTime(LPC_RTC, RTC_TIMETYPE_DAYOFMONTH, day);
}

void RTC::setMonth(uint8_t month)
{
	month %= 13;
	if(month == 0)
		month = 1;
	RTC_SetTime(LPC_RTC, RTC_TIMETYPE_MONTH, month);
}

void RTC::setYear(uint16_t year)
{
	year %= 4096;
	RTC_SetTime(LPC_RTC, RTC_TIMETYPE_YEAR, year);
}

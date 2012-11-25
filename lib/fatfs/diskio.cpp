/*
 *  Copyright (c) 2012 <benemorius@gmail.com>
 * 
 *  Permission is hereby granted, free of charge, to any person
 *  obtaining a copy of this software and associated documentation
 *  files (the "Software"), to deal in the Software without
 *  restriction, including without limitation the rights to use,
 *  copy, modify, merge, publish, distribute, sublicense, and/or sell
 *  copies of the Software, and to permit persons to whom the
 *  Software is furnished to do so, subject to the following
 *  conditions:
 * 
 *  The above copyright notice and this permission notice shall be
 *  included in all copies or substantial portions of the Software.
 * 
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 *  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 *  OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 *  NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 *  HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 *  WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 *  FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 *  OTHER DEALINGS IN THE SOFTWARE.
 */

#include "diskio.h"
#include "ff.h"
#include "FatFS.h"
#include <lpc17xx_rtc.h>

DSTATUS disk_initialize(BYTE volume)
{
	return FatFS::disk_initialise(volume);
}

DSTATUS disk_status(BYTE volume)
{
	return FatFS::disk_status(volume);
}

DRESULT disk_ioctl(BYTE volume, BYTE ctrl, void* buff)
{
	return FatFS::disk_ioctl(volume, ctrl, buff);
}

DRESULT disk_read(BYTE volume, BYTE* buffer, DWORD startSector, BYTE count)
{
	return FatFS::disk_read(volume, buffer, startSector, count);
}

DRESULT disk_write(BYTE volume, const BYTE* data, DWORD startSector, BYTE count)
{
	return FatFS::disk_write(volume, data, startSector, count);
}

extern "C" DWORD get_fattime()
{
	RTC_TIME_Type time;
	RTC_GetFullTime(LPC_RTC, &time);
	
	DWORD fattime = 0;
	fattime += ((time.YEAR - 1980) & 0x7f) << 25;
	fattime += time.MONTH << 21;
	fattime += time.DOM << 16;
	fattime += time.HOUR << 11;
	fattime += time.MIN << 5;
	fattime += time.SEC / 2;
	return fattime;
}

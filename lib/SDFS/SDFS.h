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

#ifndef SDCARD_H
#define SDCARD_H

#include "SPI.h"
#include "IO.h"
#include "ff.h"
#include "FatFS.h"

#include <string>

class SDFS : public FatFSDisk
{
public:
	SDFS(SPI& spi, IO& cs);
	~SDFS();

	int32_t mount(const char* mountpath);
	int32_t unmount();
	std::string getMountpath() {return this->mountpath;}

	virtual int32_t disk_initialise();
	virtual int32_t disk_status();
	virtual int32_t disk_read(uint8_t* buffer, uint32_t startSector, uint32_t count);
	virtual int32_t disk_write(const uint8_t* data, uint32_t startSector, uint32_t count);
	virtual int32_t disk_ioctl(uint8_t ctrl, void* buffer);
	
	int disk_sync();
	int disk_sectors();
	

private:
	int _cmd(int cmd, int arg);
	int _cmdx(int cmd, int arg);
	int _cmd8();
	int _cmd58();
	int initialise_card();
	int initialise_card_v1();
	int initialise_card_v2();

	int _read(char *buffer, int length);
	int _write(const char *buffer, int length);
	int _sd_sectors();
	int _sectors;

	
	SPI& spi;
	IO& cs;
	std::string mountpath;
	int cdv;
	bool isMounted;
};


#endif // SDCARD_H

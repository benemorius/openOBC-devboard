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



#ifndef FATFS_H
#define FATFS_H

#include "diskio.h"
#include "ff.h"
#include <stdint.h>
#include <vector>
#include <string>

class FatFSDisk
{
public:
	virtual int32_t disk_initialise() = 0;
	virtual int32_t disk_status() = 0;
	virtual int32_t disk_read(uint8_t* buffer, uint32_t startSector,	 uint32_t count) = 0;
	virtual int32_t disk_write(const uint8_t* data, uint32_t startSector, uint32_t count) = 0;
	virtual int32_t disk_ioctl(uint8_t ctrl,	 void* buffer) = 0;
};

class FatFS
{
public:
	static DSTATUS disk_mount(FatFSDisk* disk, const char* mountpath);
	static DSTATUS disk_umount(FatFSDisk* disk);
	static DSTATUS disk_initialise(BYTE volume);
	static DSTATUS disk_status(BYTE volume);
	static DRESULT disk_read(BYTE volume, BYTE* buffer, DWORD startSector, BYTE count);
	static DRESULT disk_write(BYTE volume, const BYTE* data, DWORD startSector, BYTE count);
	static DRESULT disk_ioctl(BYTE volume, BYTE ctrl, void* buffer);

	static int8_t volumeFromMountpath(const char* mountpath);
	static int errnoFromFResult(FRESULT result);

private:
	static std::vector<FatFSDisk*> disks;
	static std::vector<std::string> mountpaths;
	static std::vector<FATFS*> fatfsStructs;
	static std::vector<bool> fatfsVolumeNumbersInUse;
};

#endif // FATFS_H

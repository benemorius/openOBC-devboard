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

#include "FatFS.h"
#include <algorithm>
#include <cstdio>
#include <errno.h>

std::vector<FatFSDisk*> FatFS::disks;
std::vector<std::string> FatFS::mountpaths;
std::vector<FATFS*> FatFS::fatfsStructs;
std::vector<bool> FatFS::fatfsVolumeNumbersInUse;

DSTATUS FatFS::disk_mount(FatFSDisk* disk, const char* mountpath)
{
	//make sure nothing else is mounted at mountpath
	std::vector<std::string>::iterator foundMountpath = std::find(mountpaths.begin(), mountpaths.end(), mountpath);
	if(foundMountpath != mountpaths.end())
	{
		fprintf(stderr, "mount path already in use: %s\r\n", mountpath);
		return -1;
	}

	FATFS* fatfsStruct = new FATFS;
	
	//find an unused fatfs volume number to use
	uint8_t volumeNumber;
	std::vector<bool>::iterator found = std::find(fatfsVolumeNumbersInUse.begin(), fatfsVolumeNumbersInUse.end(), false);
	if(found != fatfsVolumeNumbersInUse.end()) //found an existing unused entry
	{
		volumeNumber = found - fatfsVolumeNumbersInUse.begin();
		if(f_mount(volumeNumber, fatfsStruct) != FR_OK)
		{
			delete fatfsStruct;
			return STA_NOINIT;
		}
		disks.at(volumeNumber) = disk;
		mountpaths.at(volumeNumber) = mountpath;
		fatfsStructs.at(volumeNumber) = fatfsStruct;
		fatfsVolumeNumbersInUse.at(volumeNumber) = true;
	}
	else //no unused entries - create a new one
	{
		volumeNumber = fatfsVolumeNumbersInUse.size();
		if(f_mount(volumeNumber, fatfsStruct) != FR_OK)
		{
			delete fatfsStruct;
			return STA_NOINIT;
		}
		disks.push_back(disk);
		mountpaths.push_back(mountpath);
		fatfsStructs.push_back(fatfsStruct);
		fatfsVolumeNumbersInUse.push_back(true);
	}

// 	fprintf(stderr, "mounted volume %i at %s\r\n", volumeNumber, mountpath);
	return 0;
}

DSTATUS FatFS::disk_umount(FatFSDisk* disk)
{
	std::vector<FatFSDisk*>::iterator found = std::find(FatFS::disks.begin(), FatFS::disks.end(), disk);
	if(found == disks.end())
		return STA_NOINIT;
	
	uint8_t volumeNumber = found - disks.begin();
	if(fatfsVolumeNumbersInUse.at(volumeNumber) == false)
		return STA_NOINIT;
	
	f_mount(volumeNumber, NULL);
	delete FatFS::fatfsStructs.at(volumeNumber);
	fatfsVolumeNumbersInUse.at(volumeNumber) = false;
	return 0;
}

DSTATUS FatFS::disk_initialise(BYTE volume)
{
	DSTATUS stat = 0;
	int result = FatFS::disks.at(volume)->disk_initialise();
	
	if(result == 0)
		stat = 0;
	else if(result == 3)
		stat = STA_NOINIT | STA_NODISK;
	else
		stat = STA_NOINIT;
	
	return stat;
}

DSTATUS FatFS::disk_status(BYTE volume)
{
	DSTATUS stat = 0;
	int result = FatFS::disks.at(volume)->disk_status();
	
	if(result == 0)
		stat = 0;
	else if(result == 3)
		stat = STA_NOINIT | STA_NODISK;
	else
		stat = STA_NOINIT;

	return stat;
}

DRESULT FatFS::disk_ioctl(BYTE volume, BYTE ctrl, void* buffer)
{
	DRESULT res = RES_PARERR;
	int result = FatFS::disks.at(volume)->disk_ioctl(ctrl, buffer);

	if(result == 0)
		res = RES_OK;
	
	return res;
}

DRESULT FatFS::disk_read(BYTE volume, BYTE* buffer, DWORD startSector, BYTE count)
{
	DRESULT res = RES_PARERR;
	int sectorsRead = FatFS::disks.at(volume)->disk_read(buffer, startSector, count);
	
	if(sectorsRead == count)
		res = RES_OK;
	else
		res = RES_ERROR;
	
	return res;
}

DRESULT FatFS::disk_write(BYTE volume, const BYTE* data, DWORD startSector, BYTE count)
{
	DRESULT res = RES_PARERR;
	int sectorsWritten = FatFS::disks.at(volume)->disk_write(data, startSector, count);
	
	if(sectorsWritten == count)
		res = RES_OK;
	else
		res = RES_ERROR;
	
	return res;
}

int8_t FatFS::volumeFromMountpath(const char* mountpath)
{
	std::vector<std::string>::iterator found = std::find(mountpaths.begin(), mountpaths.end(), std::string(mountpath));
	if(found == mountpaths.end())
		return -1;
	return found - mountpaths.begin();
}

int FatFS::errnoFromFResult(FRESULT result)
{
	int errornumber;
	switch(result)
	{
		case FR_OK:							/* (0) Succeeded */
			errornumber = 0;
			break;
		case FR_DISK_ERR:					/* (1) A hard error occurred in the low level disk I/O layer */
			errornumber = EIO;
			break;
		case FR_INT_ERR:					/* (2) Assertion failed */
			errornumber = EINVAL;
			break;
		case FR_NOT_READY:				/* (3) The physical drive cannot work */
			errornumber = EIO;
			break;
		case FR_NO_FILE:					/* (4) Could not find the file */
			errornumber = ENOENT;
			break;
		case FR_NO_PATH:					/* (5) Could not find the path */
			errornumber = ENOENT;
			break;
		case FR_INVALID_NAME:			/* (6) The path name format is invalid */
			errornumber = EINVAL;
			break;
		case FR_DENIED:					/* (7) Access denied due to prohibited access or directory full */
			errornumber = EACCES;
			break;
		case FR_EXIST:						/* (8) Access denied due to prohibited access */
			errornumber = EEXIST;
			break;
		case FR_INVALID_OBJECT:			/* (9) The file/directory object is invalid */
			errornumber = EFAULT;
			break;
		case FR_WRITE_PROTECTED:		/* (10) The physical drive is write protected */
			errornumber = EROFS;
			break;
		case FR_INVALID_DRIVE:			/* (11) The logical drive number is invalid */
			errornumber = ENODEV;
			break;
		case FR_NOT_ENABLED:				/* (12) The volume has no work area */
			errornumber = ENOEXEC;
			break;
		case FR_NO_FILESYSTEM:			/* (13) There is no valid FAT volume */
			errornumber = ENFILE;
			break;
		case FR_MKFS_ABORTED:			/* (14) The f_mkfs() aborted due to any parameter error */
			errornumber = ENOEXEC;
			break;
		case FR_TIMEOUT:					/* (15) Could not get a grant to access the volume within defined period */
			errornumber = EAGAIN;
			break;
		case FR_LOCKED:					/* (16) The operation is rejected according to the file sharing policy */
			errornumber = EBUSY;
			break;
		case FR_NOT_ENOUGH_CORE:		/* (17) LFN working buffer could not be allocated */
			errornumber = ENOMEM;
			break;
		case FR_TOO_MANY_OPEN_FILES:	/* (18) Number of open files > _FS_SHARE */
			errornumber = EMFILE;
			break;
		case FR_INVALID_PARAMETER:		/* (19) Given parameter is invalid */
			errornumber = EINVAL;
			break;
		default:
			errornumber = 0;
			break;
	}
	return errornumber;
}

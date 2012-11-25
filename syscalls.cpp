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

#include "syscalls.h"
#include "Debug.h"
#include "FatFS.h"

#include <algorithm>
#include <fcntl.h>
#include <string.h>

#define FILENO_OFFSET (3) //0/1/2 are reserved for stdin/stdout/stderr

#ifdef __cplusplus
extern "C" {
#endif

extern void allocate_failed();

extern Debug* debugS;
extern char _end;
static char *heap_end;

std::vector<FIL*> files;

int fatfs_volume_translate(const char* oldname, char* newname, int maxlen)
{
	//extract the mountpath and retreive the corresponding volume number
	char* fullpath = new char[strlen(oldname) + 1];
	strcpy(fullpath, oldname);
	char* mountpath = strtok(fullpath, "/");
	if(mountpath == NULL || strchr(fullpath, '/') == NULL)
		return -1;
	int8_t volumeNumber = FatFS::volumeFromMountpath(mountpath);
	
	//generate a new filepath replacing the mountpath with its volume number
	char* remainingFilename = strtok(NULL, "");
	snprintf(newname, maxlen, "%i:/%s", volumeNumber, remainingFilename);
	delete[] fullpath;
	return 0;
}

int _kill(int pid, int sig)
{
	pid = pid; sig = sig;
	errno = EINVAL;
	return -1;
}

void _exit(int status)
{
	status = status;
	while(1);
}

int _getpid()
{
	return 1;
}

char* get_heap_end()
{
	return (char*)heap_end;
}

char* get_stack_top()
{
	return (char*) __get_MSP();
	// return (char*) __get_PSP();
}

caddr_t _sbrk(int incr)
{
	char* prev_heap_end;
	if(heap_end == 0)
	{
		heap_end = &_end;
	}
	prev_heap_end = heap_end;
	if(heap_end + incr > get_stack_top())
	{
		allocate_failed();
	}
	heap_end += incr;
	return (caddr_t)prev_heap_end;
}

int _open(const char* name, int flags, int mode)
{
	uint32_t fatfsFlags;
	if(flags & O_WRONLY)
		fatfsFlags = FA_WRITE;
	else if(flags & O_RDWR)
		fatfsFlags = FA_READ | FA_WRITE;
	else
		fatfsFlags = FA_READ;

	if(flags & O_CREAT)
	{
		if(flags & O_EXCL)
			fatfsFlags |= FA_CREATE_NEW;
		else if(flags & O_TRUNC)
			fatfsFlags |= FA_CREATE_ALWAYS;
		else
			fatfsFlags |= FA_OPEN_ALWAYS;
	}
	else
	{
		fatfsFlags |= FA_OPEN_EXISTING;
	}
	//O_APPEND is handled in libc

	int filenameLength = strlen(name) + 7;
	char* fatfsFilename = new char[filenameLength];
	if(fatfs_volume_translate(name, fatfsFilename, filenameLength) != 0)
	{
		delete[] fatfsFilename;
		errno = EINVAL;
		return -1;
	}

// 	fprintf(stderr, "_open: file:%s (%s) flags:0%o fFlags:0x%x mode:0%o\r\n", name, fatfsFilename, flags, fatfsFlags, mode);
	
	FIL* file = new FIL;
	FRESULT result = f_open(file, fatfsFilename, fatfsFlags);
	delete[] fatfsFilename;
	if(result != FR_OK)
	{
		errno = FatFS::errnoFromFResult(result);
		fprintf(stderr, "f_open failed on %s: %s (%i)\r\n", name, strerror(errno), result);
		return -1;
	}

	//find the first unused file descriptor or add a new one if necessary
	uint8_t index;
	std::vector<FIL*>::iterator found = std::find(files.begin(), files.end(), (FIL*)NULL);
	if(found != files.end())
	{
// 		*found.base() = file;
		index = found - files.begin();
		files.at(index) = file;
	}
	else
	{
		files.push_back(file);
		index = files.size() - 1;
	}

// 	fprintf(stderr, "fd: %i\r\n", index + FILENO_OFFSET);
	return index + FILENO_OFFSET;
}

int _close(int fd)
{
// 	fprintf(stderr, "_close: file:%i\r\n", fd);
	if(fd < 0)
	{
		errno = EBADF;
		return -1;
	}
	if(fd < FILENO_OFFSET)
		return 0;
	if(fd > FILENO_OFFSET - 1 + files.size())
	{
		errno = EBADF;
		return -1;
	}
	fd = fd - FILENO_OFFSET;

	FRESULT result = f_close(files.at(fd));
	delete files.at(fd);
	files.at(fd) = NULL;
	if(result != FR_OK)
	{
		errno = FatFS::errnoFromFResult(result);
		fprintf(stderr, "f_close failed: %s (%i)\r\n", strerror(errno), result);
		return -1;
	}
	return 0;
}

int _fstat(int fd, struct stat *st)
{
	fd = fd;
	st->st_mode = S_IFCHR;

	return 0;
}

int fsync(int fd)
{
	FRESULT result = f_sync(files.at(fd - FILENO_OFFSET));
	if(result != FR_OK)
	{
		errno = FatFS::errnoFromFResult(result);
		return -1;
	}
	return 0;
}

int _isatty(int fd)
{
	fd = fd;
	return 1;
}

int _link(const char* oldfilename, const char* newfilename)
{
	errno = EMLINK;
	return -1; //having >1 link is not valid for fat
}

int _unlink(const char* filename)
{
	int filenameLength = strlen(filename) + 7;
	char* fatfsFilename = new char[filenameLength];
	if(fatfs_volume_translate(filename, fatfsFilename, filenameLength) != 0)
	{
		delete[] fatfsFilename;
		errno = EINVAL;
		return -1;
	}
	
	FRESULT result = f_unlink(fatfsFilename);
	delete[] fatfsFilename;
	if(result != FR_OK)
	{
		errno = FatFS::errnoFromFResult(result);
		return -1;
	}
	return 0;
}

int rename(const char* oldFilename, const char* newFilename)
{
	int ret = -1;

	//convert mountpaths to drive numbers for fatfs - "/sd/somefile" may become "2:/somefile"
	int oldFilenameLength = strlen(oldFilename) + 7;
	int newFilenameLength = strlen(newFilename) + 7;
	char* oldFatfsFilename = new char[oldFilenameLength];
	char* newFatfsFilename = new char[newFilenameLength];
	if(fatfs_volume_translate(oldFilename, oldFatfsFilename, oldFilenameLength) != 0)
	{
		delete[] oldFatfsFilename;
		delete[] newFatfsFilename;
		errno = EINVAL;
		return -1;
	}
	if(fatfs_volume_translate(newFilename, newFatfsFilename, newFilenameLength) != 0)
	{
		delete[] oldFatfsFilename;
		delete[] newFatfsFilename;
		errno = EINVAL;
		return -1;
	}

	//f_rename requires that the drive number be removed from the destination - "2:/somefile" may become "/somefile"
	FRESULT result = f_rename(oldFatfsFilename, strchr(newFatfsFilename, '/'));
	if(result == FR_OK)
		ret = 0;
	else
	{
		errno = FatFS::errnoFromFResult(result);
		ret = -1;
	}

	delete[] oldFatfsFilename;
	delete[] newFatfsFilename;
	return ret;
}

int _lseek(int fd, int offset, int whence)
{
// 	fprintf(stderr, "_lseek: file:%i offset:%i whence:%i\r\n", fd, offset, whence);
	if(fd < 0)
	{
		errno = EBADF;
		return -1;
	}
	if(fd < FILENO_OFFSET)
		return 0;
	if(fd > FILENO_OFFSET - 1 + files.size())
	{
		errno = EBADF;
		return -1;
	}
	fd = fd - FILENO_OFFSET;
	
	switch(whence)
	{
		case SEEK_SET :
		{
			offset = offset;
			break;
		}
		case SEEK_CUR :
		{
			offset = f_tell(files.at(fd)) + offset;
			break;
		}
		case SEEK_END :
		{
			offset = f_size(files.at(fd)) - offset;
			break;
		}
	}

	FRESULT result = f_lseek(files.at(fd), offset);
	//TODO verify offset was changed successfully
	
	if(result != FR_OK)
	{
		errno = FatFS::errnoFromFResult(result);
		return -1;
	}
	return 0;
}

int _read(int fd, char* buffer, int count)
{
// 	fprintf(stderr, "_read: file:%i data:%x count:%i\r\n", fd, buffer, count);
	if(fd < 0)
	{
		errno = EBADF;
		return -1;
	}
	if(fd < FILENO_OFFSET)
		return 0;
	if(fd > FILENO_OFFSET - 1 + files.size())
	{
		errno = EBADF;
		return -1;
	}
	fd = fd - FILENO_OFFSET;

	UINT bytesRead;
	FRESULT result = f_read(files.at(fd), buffer, count, &bytesRead);
	if(result != FR_OK)
	{
		errno = FatFS::errnoFromFResult(result);
		return -1;
	}
	return bytesRead;
}

int _write(int fd, char* data, int count)
{
	if(fd == fileno(stdout) || fd == fileno(stderr))
	{
		debugS->puts((uint8_t*)data, count);
		return count;
	}
	
// 	char buf[128];
// 	sprintf(buf, "_write file:%i data:%x count:%i\r\n", fd, data, count);
// 	debugS->puts(buf);

	if(fd < 0)
	{
		errno = EBADF;
		return -1;
	}
	if(fd < FILENO_OFFSET)
		return count;
	if(fd > FILENO_OFFSET - 1 + files.size())
	{
		errno = EBADF;
		return -1;
	}
	fd = fd - FILENO_OFFSET;

	uint bytesWritten;
	FRESULT result = f_write(files.at(fd), data, count, &bytesWritten);
	if(result != FR_OK)
	{
		errno = FatFS::errnoFromFResult(result);
		fprintf(stderr, "f_write failed: %s (%i)\r\n", strerror(errno), result);
		return -1;
	}
	return bytesWritten;
}

#ifdef __cplusplus
}
#endif

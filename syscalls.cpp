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

// #include <lpc17xx_uart.h>


#ifdef __cplusplus
extern "C" {
#endif

extern Debug* debugS;
// #include <stdio.h>

#undef errno
extern int errno;

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


extern char _end;
static char *heap_end;

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
#if 1
	if(heap_end + incr > get_stack_top())
	{
// 		abort();
		extern void allocate_failed();
		allocate_failed();
	}
#endif
	heap_end += incr;
	return (caddr_t)prev_heap_end;
}

int _open(char* filepath)
{
// 	printf("open:%s\r\n", filepath);
	return -1;
}

int _close(int file)
{
	file = file;
// 	printf("close:%i\r\n", file);
	return -1;
}

int _fstat(int file, struct stat *st)
{
	file = file;
	st->st_mode = S_IFCHR;

// 	debugS->puts("fstat\r\n");
	return 0;
}

int _isatty(int file)
{
	file = file;
	return 1;
}

int _lseek(int file, int ptr, int dir)
{
	file = file;
	ptr = ptr;
	dir = dir;

// 	printf("lseek:%i %i %i\r\n", file, ptr, dir);
	return 0;
}

int _read(int file, char *ptr, int len)
{
	file = file;
	ptr = ptr;
	len = len;
	ptr = ptr;

// 	printf("read:%i %i %i\r\n", file, ptr, len);
	
	return 0;

}

int _write(int file, char *ptr, int len)
{
	ptr = ptr;
	file = file;
// 	char buf[16];
// 	sprintf(buf, "<%i %i %i>", file, ptr, len);
// 	debugS->puts(buf);
	debugS->puts((uint8_t*)ptr, len);
	return len;
}


#ifdef __cplusplus
}
#endif
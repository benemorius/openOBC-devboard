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


#ifndef CALLBACK_H
#define CALLBACK_H

#include <FunctionPointer.h>
#include <Timer.h>
#include <deque>
#include <sys/types.h>

class Callback
{

public:
    Callback();
    ~Callback();
	void task();
	
	
	template<typename T>
	uint32_t addCallback(T* classPointer, void (T::*methodPointer)(void), uint32_t milliseconds)
	{
		if((methodPointer != 0) && (classPointer != 0))
		{
			functions.push_back(new FunctionPointer<void>);
			functions.back()->attach(classPointer, methodPointer);
			
			timers.push_back(new Timer);
			times.push_back(milliseconds);
		}
	}
	template<typename T>
	uint32_t addRepeatingCallback(T* classPointer, void (T::*methodPointer)(void), uint32_t milliseconds)
	{
		if((methodPointer != 0) && (classPointer != 0))
		{
			functions.push_back(new FunctionPointer<void>);
			functions.back()->attach(classPointer, methodPointer);
			
			timers.push_back(new Timer);
			times.push_back(milliseconds);;
		}
	}
	uint32_t addCallback(void (*functionPointer)(void))
	{
		functions.push_back(new FunctionPointer<void>);
		functions.back()->attach(functionPointer);
	}
	void deleteCallback(uint32_t pointer)
	{
		//FIXME TODO
	}
	
private:
	std::deque<FunctionPointer<void>*> functions;
	std::deque<Timer*> timers;
	std::deque<uint32_t> times;
};

#endif // CALLBACK_H

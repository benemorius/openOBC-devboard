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


#include "Callback.h"

Callback::Callback()
{

}

Callback::~Callback()
{
	std::deque<FunctionPointer<void>*>::iterator function = functions.begin();
	std::deque<Timer*>::iterator timer = timers.begin();
	std::deque<uint32_t>::iterator time = times.begin();
	
	while(function != functions.end())
	{
		delete *function;
		delete * timer;
		++function;
		++timer;
	}
}

void Callback::task()
{
	std::deque<FunctionPointer<void>*>::iterator function = functions.begin();
	std::deque<Timer*>::iterator timer = timers.begin();
	std::deque<uint32_t>::iterator time = times.begin();
	
	while(function != functions.end())
	{
		if((*timer)->read_ms() >= *time)
		{
			FunctionPointer<void>* f = *function;
			Timer* t = *timer;
			functions.erase(function);
			timers.erase(timer);
			times.erase(time);
			
			f->call();
			delete f;
			delete t;
			
			function = functions.begin();
			timer = timers.begin();
			time = times.begin();
		}
		else
		{
			++function;
			++timer;
			++time;
		}
	}
}


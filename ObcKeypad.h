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

#ifndef OBCKEYPAD_H
#define OBCKEYPAD_H
#include "IO.h"
#include "Input.h"
#include "FunctionPointer.h"
#include "InterruptManager.h"
#include "Timer.h"

#define BUTTON_1000_MASK (1<<4)
#define BUTTON_100_MASK (1<<0)
#define BUTTON_10_MASK (1<<12)
#define BUTTON_1_MASK (1<<16)
#define BUTTON_CONSUM_MASK (1<<5)
#define BUTTON_TEMP_MASK (1<<00000000000000000000000000000111) //value not yet known... need to fix my TEMP button :|
#define BUTTON_SPEED_MASK (1<<11)
#define BUTTON_DIST_MASK (1<<2)
#define BUTTON_CHECK_MASK (1<<6)
#define BUTTON_RANGE_MASK (1<<13)
#define BUTTON_CODE_MASK (1<<17)
#define BUTTON_LIMIT_MASK (1<<8)
#define BUTTON_TIMER_MASK (1<<18)
#define BUTTON_KMMLS_MASK (1<<14)
#define BUTTON_CLOCK_MASK (1<<3)
#define BUTTON_DATE_MASK (1<<7)
#define BUTTON_MEMO_MASK (1<<19)
#define BUTTON_SET_MASK (1<<15)

typedef enum
{
	BUTTON_1000 = 0,
	BUTTON_100,
	BUTTON_10,
	BUTTON_1,
	BUTTON_CONSUM,
	BUTTON_TEMP,
	BUTTON_SPEED,
	BUTTON_DIST,
	BUTTON_CHECK,
	BUTTON_RANGE,
	BUTTON_CODE,
	BUTTON_LIMIT,
	BUTTON_TIMER,
	BUTTON_KMMLS,
	BUTTON_CLOCK,
	BUTTON_DATE,
	BUTTON_MEMO,
	BUTTON_SET,

	BUTTON_NUM_VALUES = 18
} ObcKeypadButton_Type;

class ObcKeypad
{
public:
	ObcKeypad(IO& x0, IO& x1, IO& x2, IO& x3, IO& x4, Input& y0, Input& y1, Input& y2, Input& y3, InterruptManager& interruptManager);

	 uint32_t getKeys();
	 void scan();


	 template<typename T>
	 void attach(ObcKeypadButton_Type button, T* classPointer, void (T::*methodPointer)(void))
	 {
		 if((methodPointer != 0) && (classPointer != 0))
		 {
			 if(!callbacks[button])
				 callbacks[button] = new FunctionPointer();
			 callbacks[button]->attach(classPointer, methodPointer);
		 }
	 }
	 void attach(ObcKeypadButton_Type button, void (*functionPointer)(void))
	 {
		 if(!callbacks[button])
			 callbacks[button] = new FunctionPointer;
		 callbacks[button]->attach(functionPointer);
	 }

	 

private:
	IO& x0;
	IO& x1;
	IO& x2;
	IO& x3;
	IO& x4;
	Input& y0;
	Input& y1;
	Input& y2;
	Input& y3;
	InterruptManager& interruptManager;
	Timer debounce;
	
	FunctionPointer* callbacks[BUTTON_NUM_VALUES];

	void interruptHandler();
	
	void call(ObcKeypadButton_Type button)
	{
		if(callbacks[button] != 0)
		{
			callbacks[button]->call();
		}
	}
	 
};

#endif // OBCKEYPAD_H

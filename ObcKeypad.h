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

#define BUTTON_1000_MASK (0x10)
#define BUTTON_100_MASK (0x1)
#define BUTTON_10_MASK (0x1000)
#define BUTTON_1_MASK (0x10000)
#define BUTTON_CONSUM_MASK (0x20)
#define BUTTON_RANGE_MASK (0x2000)
#define BUTTON_TEMP_MASK (0x2)
#define BUTTON_CODE_MASK (0x20000)
#define BUTTON_SPEED_MASK (0x800)
#define BUTTON_LIMIT_MASK (0x100)
#define BUTTON_DIST_MASK (0x4)
#define BUTTON_TIMER_MASK (0x40000)
#define BUTTON_CHECK_MASK (0x40)
#define BUTTON_KMMLS_MASK (0x4000)
#define BUTTON_CLOCK_MASK (0x8)
#define BUTTON_DATE_MASK (0x80)
#define BUTTON_MEMO_MASK (0x80000)
#define BUTTON_SET_MASK (0x8000)

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


	 template<typename ClassType>
	 void attach(ObcKeypadButton_Type button, ClassType* classPointer, void (ClassType::*methodPointer)())
	 {
		 if((methodPointer != 0) && (classPointer != 0))
		 {
			 if(!callbacks[button])
				 callbacks[button] = new FunctionPointer<void, void>();
			 callbacks[button]->attach(classPointer, methodPointer);
		 }
	 }
	 void attach(ObcKeypadButton_Type button, void (*functionPointer)())
	 {
		 if(!callbacks[button])
			 callbacks[button] = new FunctionPointer<void, void>;
		 callbacks[button]->attach(functionPointer);
	 }
	 
	 template<typename ClassType>
	 void attachRaw(ClassType* classPointer, void (ClassType::*methodPointer)(uint32_t))
	 {
		 if((methodPointer != 0) && (classPointer != 0))
		 {
			 callbackRaw.attach(classPointer, methodPointer);
		 }
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
	Timer* debounce;
	Timer* interruptTimer;
	
	FunctionPointer<void, void>* callbacks[BUTTON_NUM_VALUES];
	FunctionPointer<void, uint32_t> callbackRaw;

	void interruptHandler();
	
	void call(ObcKeypadButton_Type button)
	{
		if(callbacks[button] != 0)
		{
			callbacks[button]->call();
		}
	}
	 
protected:
    uint32_t activeKeys;
};

#endif // OBCKEYPAD_H

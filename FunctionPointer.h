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

#ifndef FUNCTIONPOINTER_H
#define FUNCTIONPOINTER_H

#include <stdint.h>

class FP
{
public:
	virtual void call() = 0;
};

template<typename Type>
class FPointer : public FP
{
public:
	FPointer()
	{
		this->objectP = 0;
		this->methodP = 0;
	}

	void attach(Type* objectP, void (Type::*methodP)(void))
	{
		if((objectP != 0) && (methodP != 0))
		{
			this->objectP = objectP;
			this->methodP = methodP;
		}
	}

	virtual void call()
	{
		if(objectP != 0 && methodP != 0)
		{
			(objectP->*methodP)();
		}
	}


private:
	Type* objectP;
	void (Type::*methodP)(void);
	
};



class FunctionPointer
{
public:
	FunctionPointer()
	{
		this->fp = 0;
		this->function = 0;
	}
	
	template<typename T>
	void attach(T* objectP, void (T::*methodP)(void))
	{
		if(this->fp != 0)
		{
			delete this->fp; //TODO allow multiple attachments and detachments
			this->fp = 0;
		}
		
		FPointer<T>* fp = new FPointer<T>();
		fp->attach(objectP, methodP);
		this->fp = fp;
	}
	void attach(void (*function)(void))
	{
		this->function = function;
		if(function == 0 && fp != 0)
		{
			delete fp; //TODO allow multiple attachments and detachments
			fp = 0;
		}
	}
	
	void call()
	{
		if(function != 0)
		{
			function();
		}
		if(fp != 0)
		{
			fp->call();
		}
	}

	bool isValid()
	{
		if(function != 0 || fp != 0)
			return true;
		else
			return false;
	}

private:
	FP* fp;
	void (*function)(void);
	
};

#endif // FUNCTIONPOINTER_H

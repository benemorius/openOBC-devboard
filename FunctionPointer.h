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
#include <vector>

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

	bool operator==(const FPointer& rhs) const
	{
		if(rhs.objectP == this->objectP && rhs.methodP == this->methodP)
			return true;
		else
			return false;
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
		
	}
	
	template<typename T>
	void attach(T* objectP, void (T::*methodP)(void))
	{
		if(objectP != 0 && methodP != 0)
		{
			FPointer<T>* fp = new FPointer<T>();
			fp->attach(objectP, methodP);
			methodPointers.push_back(fp);
		}
	}
	template<typename T>
	void detach(T* objectP, void (T::*methodP)(void))
	{
		for(uint32_t numMethodPointers = methodPointers.size(); numMethodPointers;)
		{
			numMethodPointers--;
			FPointer<T>* fp = new FPointer<T>();
			fp->attach(objectP, methodP);
			if(*(FPointer<T>*)(methodPointers[numMethodPointers]) == *fp)
			{
				delete methodPointers[numMethodPointers];
				methodPointers.erase(methodPointers.begin() + numMethodPointers);
			}
			delete fp;
		}
	}

	void attach(void (*function)(void))
	{
		if(function != 0)
			functionPointers.push_back(function);
	}
	void detach(void (*function)(void))
	{
		for(uint32_t numFunctionPointers = functionPointers.size(); numFunctionPointers;)
		{
			numFunctionPointers--;
			if(functionPointers[numFunctionPointers] == function)
				functionPointers.erase(functionPointers.begin() + numFunctionPointers);
		}
	}

	void call()
	{
		uint32_t numFunctionPointers = functionPointers.size();
		while(numFunctionPointers--)
		{
			functionPointers[numFunctionPointers]();
		}
		
		uint32_t numMethodPointers = methodPointers.size();
		while(numMethodPointers--)
		{
			methodPointers[numMethodPointers]->call();
		}
	}

	bool isValid()
	{
		if(functionPointers.size() != 0 || methodPointers.size() != 0)
			return true;
		else
			return false;
	}

private:
	std::vector<FP*> methodPointers;
	std::vector<void (*)(void)> functionPointers;
	
};

#endif // FUNCTIONPOINTER_H

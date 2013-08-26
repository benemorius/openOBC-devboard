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

template<typename ReturnType, typename ArgType>
class _FunctionPointer
{
public:
	virtual ReturnType call(ArgType) = 0;
};

template<typename ReturnType>
class _FunctionPointer<ReturnType, void>
{
public:
	virtual ReturnType call(void) = 0;
};

template<typename ObjectType, typename ReturnType, typename ArgType>
class _MethodPointer : public _FunctionPointer<ReturnType, ArgType>
{
public:
	_MethodPointer()
	{
		this->objectP = 0;
		this->methodP = 0;
	}

	void attach(ObjectType* objectP, ReturnType (ObjectType::*methodP)(ArgType))
	{
		if((objectP != 0) && (methodP != 0))
		{
			this->objectP = objectP;
			this->methodP = methodP;
		}
	}

	virtual ReturnType call(ArgType arg)
	{
		if(objectP != 0 && methodP != 0)
		{
			return (objectP->*methodP)(arg);
		}
	}

	bool operator==(const _MethodPointer& rhs) const
	{
		if(rhs.objectP == this->objectP && rhs.methodP == this->methodP)
			return true;
		else
			return false;
	}

private:
	ObjectType* objectP;
	ReturnType (ObjectType::*methodP)(ArgType);

};

template<typename ObjectType, typename ReturnType>
class _MethodPointer<ObjectType, ReturnType, void> : public _FunctionPointer<ReturnType, void>
{
public:
	_MethodPointer()
	{
		this->objectP = 0;
		this->methodP = 0;
	}
	
	void attach(ObjectType* objectP, ReturnType (ObjectType::*methodP)(void))
	{
		if((objectP != 0) && (methodP != 0))
		{
			this->objectP = objectP;
			this->methodP = methodP;
		}
	}
	
	virtual ReturnType call(void)
	{
		if(objectP != 0 && methodP != 0)
		{
			return (objectP->*methodP)();
		}
	}
	
	bool operator==(const _MethodPointer& rhs) const
	{
		if(rhs.objectP == this->objectP && rhs.methodP == this->methodP)
			return true;
		else
			return false;
	}
	
private:
	ObjectType* objectP;
	ReturnType (ObjectType::*methodP)(void);
	
};

template<typename ReturnType, typename ArgType>
class FunctionPointer
{
public:
	
	~FunctionPointer()
	{
		for(uint32_t numMethodPointers = methodPointers.size(); numMethodPointers;)
		{
			numMethodPointers--;
			delete methodPointers[numMethodPointers];
		}
	}
	
	template<typename ObjectType>
	void attach(ObjectType* objectP, ReturnType (ObjectType::*methodP)(ArgType))
	{
		if(objectP != 0 && methodP != 0)
		{
			_MethodPointer<ObjectType, ReturnType, ArgType>* fp = new _MethodPointer<ObjectType, ReturnType, ArgType>();
			fp->attach(objectP, methodP);
			methodPointers.push_back(fp);
		}
	}
	template<typename ObjectType>
	void detach(ObjectType* objectP, ReturnType (ObjectType::*methodP)())
	{
		for(uint32_t numMethodPointers = methodPointers.size(); numMethodPointers;)
		{
			numMethodPointers--;
			_MethodPointer<ObjectType, ReturnType, ArgType>* fp = new _MethodPointer<ObjectType, ReturnType, ArgType>();
			fp->attach(objectP, methodP);
			if(*(_MethodPointer<ObjectType, ReturnType, ArgType>*)(methodPointers[numMethodPointers]) == *fp)
			{
				delete methodPointers[numMethodPointers];
				methodPointers.erase(methodPointers.begin() + numMethodPointers);
			}
			delete fp;
		}
	}

	void attach(ReturnType (*function)(ArgType))
	{
		if(function != 0)
			functionPointers.push_back(function);
	}
	void detach(ReturnType (*function)(ArgType))
	{
		for(uint32_t numFunctionPointers = functionPointers.size(); numFunctionPointers;)
		{
			numFunctionPointers--;
			if(functionPointers[numFunctionPointers] == function)
				functionPointers.erase(functionPointers.begin() + numFunctionPointers);
		}
	}
	void detachAll()
	{
		for(uint32_t numMethodPointers = methodPointers.size(); numMethodPointers;)
		{
			numMethodPointers--;
			delete methodPointers[numMethodPointers];
			methodPointers.erase(methodPointers.begin() + numMethodPointers);
		}
		for(uint32_t numFunctionPointers = functionPointers.size(); numFunctionPointers;)
		{
			numFunctionPointers--;
			functionPointers.erase(functionPointers.begin() + numFunctionPointers);
		}
	}

	ReturnType call(ArgType arg)
	{
		uint32_t numFunctionPointers = functionPointers.size();
		if(numFunctionPointers == 1)
			return functionPointers[0](arg);
		
		while(numFunctionPointers--)
		{
			functionPointers[numFunctionPointers](arg);
		}
		
		uint32_t numMethodPointers = methodPointers.size();
		if(numMethodPointers == 1)
			return methodPointers[0]->call(arg);
		
		while(numMethodPointers--)
		{
			methodPointers[numMethodPointers]->call(arg);
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
	std::vector<_FunctionPointer<ReturnType, ArgType>*> methodPointers;
	std::vector<ReturnType (*)(ArgType)> functionPointers;
	
};

template<typename ReturnType>
class FunctionPointer<ReturnType, void>
{
public:
	
	~FunctionPointer()
	{
		for(uint32_t numMethodPointers = methodPointers.size(); numMethodPointers;)
		{
			numMethodPointers--;
			delete methodPointers[numMethodPointers];
		}
	}
	
	template<typename ObjectType>
	void attach(ObjectType* objectP, ReturnType (ObjectType::*methodP)(void))
	{
		if(objectP != 0 && methodP != 0)
		{
			_MethodPointer<ObjectType, ReturnType, void>* fp = new _MethodPointer<ObjectType, ReturnType, void>();
			fp->attach(objectP, methodP);
			methodPointers.push_back(fp);
		}
	}
	template<typename ObjectType>
	void detach(ObjectType* objectP, ReturnType (ObjectType::*methodP)())
	{
		for(uint32_t numMethodPointers = methodPointers.size(); numMethodPointers;)
		{
			numMethodPointers--;
			_MethodPointer<ObjectType, ReturnType, void>* fp = new _MethodPointer<ObjectType, ReturnType, void>();
			fp->attach(objectP, methodP);
			if(*(_MethodPointer<ObjectType, ReturnType, void>*)(methodPointers[numMethodPointers]) == *fp)
			{
				delete methodPointers[numMethodPointers];
				methodPointers.erase(methodPointers.begin() + numMethodPointers);
			}
			delete fp;
		}
	}
	
	void attach(ReturnType (*function)(void))
	{
		if(function != 0)
			functionPointers.push_back(function);
	}
	void detach(ReturnType (*function)(void))
	{
		for(uint32_t numFunctionPointers = functionPointers.size(); numFunctionPointers;)
		{
			numFunctionPointers--;
			if(functionPointers[numFunctionPointers] == function)
				functionPointers.erase(functionPointers.begin() + numFunctionPointers);
		}
	}
	void detachAll()
	{
		for(uint32_t numMethodPointers = methodPointers.size(); numMethodPointers;)
		{
			numMethodPointers--;
			delete methodPointers[numMethodPointers];
			methodPointers.erase(methodPointers.begin() + numMethodPointers);
		}
		for(uint32_t numFunctionPointers = functionPointers.size(); numFunctionPointers;)
		{
			numFunctionPointers--;
			functionPointers.erase(functionPointers.begin() + numFunctionPointers);
		}
	}
	
	ReturnType call(void)
	{
		uint32_t numFunctionPointers = functionPointers.size();
		if(numFunctionPointers == 1)
			return functionPointers[0]();
		
		while(numFunctionPointers--)
		{
			bool isLast = (numFunctionPointers == 0) ? true : false;
			functionPointers[numFunctionPointers]();
		}
		
		uint32_t numMethodPointers = methodPointers.size();
		if(numMethodPointers == 1)
			return methodPointers[0]->call();
		
		while(numMethodPointers--)
		{
			bool isLast = (numMethodPointers == 0) ? true : false;
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
	std::vector<_FunctionPointer<ReturnType, void>*> methodPointers;
	std::vector<ReturnType (*)(void)> functionPointers;
	
};

#endif // FUNCTIONPOINTER_H

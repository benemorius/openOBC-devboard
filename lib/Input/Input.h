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

#ifndef INPUT_H
#define INPUT_H

#include <lpc_types.h>

class Input
{
public:
    Input(uint8_t port, uint8_t pin, bool onIsHigh = true);
	 bool getState() const;
	 void setPullup();
	 void setPulldown();
	 void setTristate();
	 
	 uint8_t getPort() const {return port;}
	 uint8_t getPin() const {return pin;}
	 
	 operator bool() const { return getState();}


private:
	uint8_t port;
	uint8_t pin;
	bool onIsHigh;
};

#endif // INPUT_H

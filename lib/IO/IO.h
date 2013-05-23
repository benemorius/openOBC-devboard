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

#ifndef IO_H
#define IO_H
#include <stdint.h>

class IO
{
public:
	IO(uint8_t port, uint8_t pin, bool isOn = false, bool onIsHigh = true);
	
	virtual void setState(bool on);
	virtual bool getState() const;
	void on();
	void off();
	void toggle();
	virtual void setOpenDrain(bool isOpenDrain);
	virtual void setInput();
	virtual void setOutput();
	virtual void setPullup();
	virtual void setPulldown();
	virtual void setTristate();
	void setOnIsHigh(bool onIsHigh) {this->onIsHigh = onIsHigh;}
	bool getOnIsHigh() const {return this->onIsHigh;}

	uint8_t getPort() const {return port;}
	uint8_t getPin() const {return pin;}
	
	IO& operator=(bool state);
	IO& operator=(IO& io);
	operator bool() const { return getState();}
	
protected:
	bool onIsHigh;
	bool isOn;
	
private:
	uint8_t port;
	uint8_t pin;
	bool isOpenDrain;

};

#endif // IO_H

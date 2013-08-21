/*
    Copyright (c) 2013 <benemorius@gmail.com>

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

#ifndef PCA95XXPIN_H
#define PCA95XXPIN_H

#include "PCA95xx.h"
#include <IO.h>

class PCA95xxPin : public IO
{
public:
	PCA95xxPin(PCA95xx& pca, uint8_t port, uint8_t pin, bool isOutput = false, bool isOn = false, bool onIsHigh = true);
	~PCA95xxPin();
	
	void setState(bool state);
	bool getState() const;
	void setOpenDrain(bool isOpenDrain) {}; //NOT SUPPORTED
	void setInput();
	void setOutput();
	void setPullup() {}; //NOT SUPPORTED
	void setPulldown() {}; //NOT SUPPORTED
	void setTristate() {}; //NOT SUPPORTED
	
private:
	PCA95xx& pca;
	uint8_t port;
	uint8_t pin;
	bool state;
	uint16_t bitmask;
	bool isOutput;
	
};
#endif // PCA95XXPIN_H

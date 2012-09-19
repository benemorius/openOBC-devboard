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

#ifndef UART_H
#define UART_H

#include "FunctionPointer.h"

#include <lpc_types.h>
#include <lpc17xx_uart.h>
#include <stddef.h>
#include "InterruptManager.h"


#define RB_SIZE (256)
#define RB_MASK (RB_SIZE - 1)

class Uart
{
public:
	Uart(int8_t txPort, int8_t txPin, int8_t rxPort, int8_t rxPin, uint32_t baud, UART_PARITY_Type parity, InterruptManager* interruptManager);

	bool available();
	bool readable();
	bool writable();
	void begin(int32_t baud);
	void listen();
	void flush();
	void setBaud(uint32_t baud);

	size_t getdata(uint8_t* rxbuf, uint32_t buflen);
	char getc();
	size_t putc(char c);
	size_t puts(const char* string);
	size_t puts(const void* txbuf, uint32_t buflen);
	size_t printf(const char* format, ...);
	

	void receiveHandler();


	template<typename T>
	void attach(T* classPointer, void (T::*methodPointer)(void))
	{
		if((methodPointer != 0) && (classPointer != 0))
		{
			callback.attach(classPointer, methodPointer);
		}
	}
	void attach(void (*functionPointer)(void))
	{
		callback.attach(functionPointer);
	}
	

private:
	LPC_UART_TypeDef* uartPeripheral;
	int8_t txPort;
	int8_t txPin;
	int8_t rxPort;
	int8_t rxPin;
	volatile uint32_t bufferWriteP;
	volatile uint32_t bufferReadP;
	char ringBuffer[RB_SIZE];
	uint32_t baud;
	FunctionPointer callback;
	

};


#endif // UART_H

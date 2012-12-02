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

#include "Uart.h"
#include "board.h"

#include <lpc17xx_uart.h>
#include <lpc17xx_pinsel.h>
#include <lpc17xx_gpio.h>

#include <string.h>
#include <stdarg.h>
#include <cstdio>


Uart::Uart(int8_t txPort, int8_t txPin, int8_t rxPort, int8_t rxPin, uint32_t baud, UART_PARITY_Type parity, InterruptManager* interruptManager)
{
	this->txPort = txPort;
	this->txPin = txPin;
	this->rxPort = rxPort;
	this->rxPin = rxPin;
	this->baud = baud;

	PINSEL_CFG_Type PinCfg;
	if(txPort == 0 && txPin == 0)
	{
		PinCfg.Funcnum = 2;
		this->uartPeripheral = (LPC_UART_TypeDef*)LPC_UART3;
	}
	else if(txPort == 0 && txPin == 2)
	{
		PinCfg.Funcnum = 1;
		this->uartPeripheral = (LPC_UART_TypeDef*)LPC_UART0;
	}
	else if(txPort == 0 && txPin == 10)
	{
		PinCfg.Funcnum = 1;
		this->uartPeripheral = (LPC_UART_TypeDef*)LPC_UART2;
	}
	else if(txPort == 0 && txPin == 15)
	{
		PinCfg.Funcnum = 1;
		this->uartPeripheral = (LPC_UART_TypeDef*)LPC_UART1;
	}
	else if(txPort == 0 && txPin == 25)
	{
		PinCfg.Funcnum = 3;
		this->uartPeripheral = (LPC_UART_TypeDef*)LPC_UART3;
	}
	else if(txPort == 2 && txPin == 0)
	{
		PinCfg.Funcnum = 2;
		this->uartPeripheral = (LPC_UART_TypeDef*)LPC_UART1;
	}
	else if(txPort == 2 && txPin == 8)
	{
		PinCfg.Funcnum = 2;
		this->uartPeripheral = (LPC_UART_TypeDef*)LPC_UART2;
	}
	else if(txPort == 4 && txPin == 28)
	{
		PinCfg.Funcnum = 3;
		this->uartPeripheral = (LPC_UART_TypeDef*)LPC_UART3;
	}
	else
	{
		//invalid port/pin
		std::printf("invalid uart pins specified\r\n");
		while(1);
	}
	
	
	PinCfg.OpenDrain = PINSEL_PINMODE_NORMAL;
	PinCfg.Pinmode = PINSEL_PINMODE_TRISTATE;
	PinCfg.Portnum = txPort;
	PinCfg.Pinnum = txPin;
	PINSEL_ConfigPin(&PinCfg);
	PinCfg.Portnum = rxPort;
	PinCfg.Pinnum = rxPin;
	PINSEL_ConfigPin(&PinCfg);

	
	UART_CFG_Type UARTConfigStruct;
	UART_ConfigStructInit(&UARTConfigStruct);
	UARTConfigStruct.Baud_rate = baud;
	UARTConfigStruct.Parity = parity;
	UART_Init(uartPeripheral, &UARTConfigStruct);
	
	UART_FIFO_CFG_Type UARTFIFOConfigStruct;
	UART_FIFOConfigStructInit(&UARTFIFOConfigStruct);
	UART_FIFOConfig(uartPeripheral, &UARTFIFOConfigStruct);
	
	
	// Enable UART Transmit
	UART_TxCmd(uartPeripheral, ENABLE);

	//TODO set callback method in uart receive interrupt handler
// 	interruptHandler->attach(this, &Uart::receiveHandler);

	/* Enable UART Rx interrupt */
	UART_IntConfig(uartPeripheral, UART_INTCFG_RBR, ENABLE);
	/* Enable UART line status interrupt */
	UART_IntConfig(uartPeripheral, UART_INTCFG_RLS, ENABLE);


	this->bufferReadP = 0;
	this->bufferWriteP = 0;

	if(this->uartPeripheral == (LPC_UART_TypeDef*)LPC_UART0)
	{
		interruptManager->attach(IRQ_UART0, this, &Uart::receiveHandler);
		NVIC_SetPriority(UART0_IRQn, ((0x01<<3)|0x01)); //preemption = 1, sub-priority = 1
		NVIC_EnableIRQ(UART0_IRQn);
	}
	else if(this->uartPeripheral == (LPC_UART_TypeDef*)LPC_UART1)
	{
		interruptManager->attach(IRQ_UART1, this, &Uart::receiveHandler);
		NVIC_SetPriority(UART1_IRQn, ((0x01<<3)|0x01));
		NVIC_EnableIRQ(UART1_IRQn);
	}
	else if(this->uartPeripheral == (LPC_UART_TypeDef*)LPC_UART2)
	{
		interruptManager->attach(IRQ_UART2, this, &Uart::receiveHandler);
		NVIC_SetPriority(UART2_IRQn, ((0x01<<3)|0x01));
		NVIC_EnableIRQ(UART2_IRQn);
	}
	else if(this->uartPeripheral == (LPC_UART_TypeDef*)LPC_UART3)
	{
		interruptManager->attach(IRQ_UART3, this, &Uart::receiveHandler);
		NVIC_SetPriority(UART3_IRQn, ((0x01<<3)|0x01));
		NVIC_EnableIRQ(UART3_IRQn);
	}
	else
	{
		std::printf("invalid uart perhiperal specified\r\n");
		while(1);
	}
}

void Uart::begin(int32_t baud)
{
	//FIXME initialize and set baud rate
}

void Uart::setBaud(uint32_t baud)
{
	this->baud = baud;
	//FIXME change baud rate configuration in hardware
}

void Uart::listen()
{
	//TODO enable/disable receive???
}

void Uart::flush()
{
	//FIXME flush transmit buffer
}

bool Uart::available()
{
	return bufferWriteP != bufferReadP;
}

bool Uart::readable()
{
	if(bufferWriteP != bufferReadP)
		return true;
	else
		return false;
}

bool Uart::writable()
{
	return uartPeripheral->LSR & UART_LSR_THRE;
}

size_t Uart::puts(const void* txbuf, uint32_t buflen)
{
	TRANSFER_BLOCK_Type flag = BLOCKING;

	UART_Send(uartPeripheral, (uint8_t*)txbuf, buflen, flag);
	
	//wait for uart to finish sending
	while(!(uartPeripheral->LSR & UART_LSR_TEMT));
	
	return buflen;
}
size_t Uart::puts(const char* string)
{
	while(*string)
		putc(*string++);
}

size_t Uart::putc(char c)
{
	TRANSFER_BLOCK_Type flag = BLOCKING;
	UART_Send(uartPeripheral, (uint8_t*)&c, 1, flag);
	while(!(uartPeripheral->LSR & UART_LSR_TEMT));
	
}

size_t Uart::getdata(uint8_t* rxbuf, uint32_t buflen)
{
	uint8_t *data = (uint8_t *)rxbuf;
	uint32_t bytes = 0;
	
	/* Temporarily lock out UART receive interrupts during this
	 r ead so the* U*ART receive interrupt won't cause problems
	 with the index values */
	UART_IntConfig(uartPeripheral, UART_INTCFG_RBR, DISABLE);
	
	/* Loop until receive buffer ring is empty or
	 until max_by*tes* expires */
	while(buflen > 0 && bufferWriteP != bufferReadP)
	{
		/* Read data from ring buffer into user buffer */
		*data = ringBuffer[bufferReadP];
		data++;
		
		/* Update tail pointer */
		bufferReadP = (bufferReadP + 1) & RB_MASK;
		
		/* Increment data count and decrement buffer size count */
		bytes++;
		buflen--;
	}
	
	/* Re-enable UART interrupts */
	UART_IntConfig(uartPeripheral, UART_INTCFG_RBR, ENABLE);
	
	return bytes;
}

char Uart::getc()
{
	char c = 0;
	UART_IntConfig(uartPeripheral, UART_INTCFG_RBR, DISABLE);
	if(bufferWriteP != bufferReadP)
	{
		c = ringBuffer[bufferReadP];
		bufferReadP = (bufferReadP + 1) & RB_MASK;
		
	}
	UART_IntConfig(uartPeripheral, UART_INTCFG_RBR, ENABLE);
	return c;
}

size_t Uart::printf(const char* format, ... )
{
	char buffer[100];
	
	va_list args;
	va_start(args, format);
	vsprintf(buffer, format, args);
	va_end(args);
	
	return puts(buffer, strlen(buffer));
}

void Uart::receiveHandler()
{
	uint32_t intsrc, tmp, tmp1;
	
	/* Determine the interrupt source */
	intsrc = UART_GetIntId(uartPeripheral);
	tmp = intsrc & UART_IIR_INTID_MASK;
	
	// Receive Line Status
	if(tmp == UART_IIR_INTID_RLS)
	{
		// Check line status
		tmp1 = UART_GetLineStatus(uartPeripheral);
		// Mask out the Receive Ready and Transmit Holding empty status
		tmp1 &= (UART_LSR_OE | UART_LSR_PE | UART_LSR_FE | UART_LSR_BI | UART_LSR_RXFE);
		// If any error exist
		if(tmp1)
		{
			
		}
	}
	
	// Receive Data Available or Character time-out
	if((tmp == UART_IIR_INTID_RDA) || (tmp == UART_IIR_INTID_CTI))
	{
		uint8_t tmpc;
		uint32_t rLen;
		
		while(1)
		{
			// Call UART read function in UART driver
			rLen = UART_Receive((LPC_UART_TypeDef *)uartPeripheral, &tmpc, 1, NONE_BLOCKING);
			// If data received
			if(rLen)
			{
				/* Check if buffer is more space
				 * If no more space, remaining character will be trimmed out
				 */
				if((bufferWriteP & RB_MASK) + 1 != (bufferReadP & RB_MASK))
				{
					ringBuffer[bufferWriteP] = tmpc;
					bufferWriteP = (bufferWriteP) + 1 & RB_MASK;
// 					std::printf("{%c}", tmpc);
				}
				else //buffer full
				{
					std::printf("Uart::receiveHandler(): buffer overflow\r\n");
				}
			}
			// no more data
			else
			{
				break;
			}
		}
	}
	callback.call();
}


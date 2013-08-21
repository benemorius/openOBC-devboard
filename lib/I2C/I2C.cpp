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


#include "I2C.h"
#include <lpc17xx_pinsel.h>

I2C::I2C(uint8_t sdaPort, uint8_t sdaPin, uint8_t sclPort, uint8_t sclPin, uint32_t hz) : sdaPort(sdaPort), sdaPin(sdaPin), sclPort(sclPort), sclPin(sclPin), hz(hz)
{
	PINSEL_CFG_Type PinCfg;
	if(sdaPort == 0 && sdaPin == 27)
	{
		PinCfg.Funcnum = 1;
		this->peripheral = LPC_I2C0;
	}
	else if(sdaPort == 0 && sdaPin == 0)
	{
		PinCfg.Funcnum = 3;
		this->peripheral = LPC_I2C1;
	}
	else if(sdaPort == 0 && sdaPin == 19)
	{
		PinCfg.Funcnum = 3;
		this->peripheral = LPC_I2C1;
	}
	else
	{
		//invalid port/pin specified
		while(1);
	}
	PinCfg.OpenDrain = PINSEL_PINMODE_NORMAL;
	PinCfg.Pinmode = PINSEL_PINMODE_PULLUP;
	
	PinCfg.Portnum = sdaPort;
	PinCfg.Pinnum = sdaPin;
	PINSEL_ConfigPin(&PinCfg);
	PinCfg.Portnum = sclPort;
	PinCfg.Pinnum = sclPin;
	PINSEL_ConfigPin(&PinCfg);
	
	I2C_Init(this->peripheral, hz);
	I2C_Cmd(this->peripheral, ENABLE);
}

I2C::~I2C()
{
	I2C_Cmd(peripheral, DISABLE);
	I2C_DeInit(peripheral);
}

I2C& I2C::setFrequency(uint32_t hz)
{
	if(hz == this->hz)
		return *this;
	
	this->hz = hz;
// 	I2C_Cmd(peripheral, DISABLE);
// 	I2C_DeInit(peripheral);
	I2C_Init(peripheral, hz);
	I2C_Cmd(peripheral, ENABLE);
	return *this;
}

uint32_t I2C::read(uint8_t address, uint8_t* buffer, uint32_t length)
{
	I2C_M_SETUP_Type transferMCfg;
	transferMCfg.sl_addr7bit = address >> 1;
	transferMCfg.tx_length = 0;
	transferMCfg.rx_data = buffer;
	transferMCfg.rx_length = length;
	transferMCfg.retransmissions_max = 3;
	if(I2C_MasterTransferData(peripheral, &transferMCfg, I2C_TRANSFER_POLLING) == SUCCESS)
		return length;
	else
		return 0;
}

uint32_t I2C::write(uint8_t address, uint8_t* data, uint32_t length)
{
	I2C_M_SETUP_Type transferMCfg;
	transferMCfg.sl_addr7bit = address >> 1;
	transferMCfg.tx_data = data ;
	transferMCfg.tx_length =length;
	transferMCfg.rx_length = 0;
	transferMCfg.retransmissions_max = 3;
	if(I2C_MasterTransferData(peripheral, &transferMCfg, I2C_TRANSFER_POLLING) == SUCCESS)
		return length;
	else
		return 0;
}

uint32_t I2C::readwrite(uint8_t address, uint8_t* writeData, uint32_t writeLength, uint8_t* readBuffer, uint32_t readLength)
{
	I2C_M_SETUP_Type transferMCfg;
	transferMCfg.sl_addr7bit = address >> 1;
	transferMCfg.tx_data = writeData ;
	transferMCfg.tx_length =writeLength;
	transferMCfg.rx_data = readBuffer;
	transferMCfg.rx_length = readLength;
	transferMCfg.retransmissions_max = 3;
	if(I2C_MasterTransferData(peripheral, &transferMCfg, I2C_TRANSFER_POLLING) == SUCCESS)
		return readLength;
	else
		return 0;
}

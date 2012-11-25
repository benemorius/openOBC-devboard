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

#include "SPI.h"
#include <lpc17xx_pinsel.h>

SPI::SPI(uint8_t mosiPort, uint8_t mosiPin, uint8_t misoPort, uint8_t misoPin, uint8_t sckPort, uint8_t sckPin, uint32_t clockRate)
{
	this->mosiPort = mosiPort;
	this->mosiPin = mosiPin;
	this->misoPort = misoPort;
	this->misoPin = misoPin;
	this->sckPort = sckPort;
	this->sckPin = sckPin;
	this->clockRate = clockRate;

	PINSEL_CFG_Type PinCfg;
	if(mosiPort == 0 && mosiPin == 9)
	{
		PinCfg.Funcnum = 2;
		this->peripheral = LPC_SSP1;
	}
	else if(mosiPort == 0 && mosiPin == 18)
	{
		PinCfg.Funcnum = 2;
		this->peripheral = LPC_SSP0;
	}
	else if(mosiPort == 1 && mosiPin == 24)
	{
		PinCfg.Funcnum = 3;
		this->peripheral = LPC_SSP0;
	}
	else
	{
		//invalid port/pin specified
		while(1);
	}

	PinCfg.OpenDrain = PINSEL_PINMODE_NORMAL;
	PinCfg.Pinmode = PINSEL_PINMODE_PULLUP;
	
	PinCfg.Portnum = mosiPort;
	PinCfg.Pinnum = mosiPin;
	PINSEL_ConfigPin(&PinCfg);
	PinCfg.Portnum = misoPort;
	PinCfg.Pinnum = misoPin;
	PINSEL_ConfigPin(&PinCfg);
	PinCfg.Portnum = sckPort;
	PinCfg.Pinnum = sckPin;
	PINSEL_ConfigPin(&PinCfg);

	// SSP Configuration structure variable
	SSP_CFG_Type SSP_ConfigStruct;
	// initialize SSP configuration structure to default
	SSP_ConfigStructInit(&SSP_ConfigStruct);
	
	SSP_ConfigStruct.CPHA = SSP_CPHA_FIRST; //TODO make these configurable
// 	SSP_ConfigStruct.CPHA = SSP_CPHA_SECOND;
	SSP_ConfigStruct.CPOL = SSP_CPOL_HI;
// 	SSP_ConfigStruct.CPOL = SSP_CPOL_LO;
	SSP_ConfigStruct.ClockRate = this->clockRate;
	// 	SSP_ConfigStruct.DataOrder = SPI_DATA_MSB_FIRST;
	SSP_ConfigStruct.Databit = SSP_DATABIT_8;
	SSP_ConfigStruct.Mode = SSP_MASTER_MODE;
	
	// Initialize SSP peripheral with parameter given in structure above
	SSP_Init(this->peripheral, &SSP_ConfigStruct);
	
	// Enable SSP peripheral
	SSP_Cmd(this->peripheral, ENABLE);
}

void SPI::setClockRate(uint32_t hz)
{
	this->clockRate = hz;
	
	SSP_CFG_Type SSP_ConfigStruct;
	SSP_ConfigStructInit(&SSP_ConfigStruct);
	SSP_ConfigStruct.CPHA = SSP_CPHA_FIRST; //TODO make these configurable
	SSP_ConfigStruct.CPOL = SSP_CPOL_HI;
	SSP_ConfigStruct.ClockRate = this->clockRate;
	SSP_ConfigStruct.Databit = SSP_DATABIT_8;
	SSP_ConfigStruct.Mode = SSP_MASTER_MODE;
	
	SSP_Init(this->peripheral, &SSP_ConfigStruct);
	SSP_Cmd(this->peripheral, ENABLE);
}

void SPI::setPullup(bool isEnabled)
{
	PINSEL_CFG_Type PinCfg;
	
	if(mosiPort == 0 && mosiPin == 9)
		PinCfg.Funcnum = 2;
	else if(mosiPort == 0 && mosiPin == 18)
		PinCfg.Funcnum = 2;
	else if(mosiPort == 1 && mosiPin == 24)
		PinCfg.Funcnum = 3;
	else //invalid port/pin specified
		while(1);

	PinCfg.OpenDrain = PINSEL_PINMODE_NORMAL;
	PinCfg.Pinmode = isEnabled ? PINSEL_PINMODE_PULLUP : PINSEL_PINMODE_TRISTATE;
	PinCfg.Portnum = misoPort;
	PinCfg.Pinnum = misoPin;
	PINSEL_ConfigPin(&PinCfg);
}

uint8_t SPI::readWrite(uint8_t writeByte)
{
	uint8_t readByte;
	SSP_DATA_SETUP_Type xferConfig;
	xferConfig.tx_data = &writeByte;
	xferConfig.rx_data = &readByte;
	xferConfig.length = 1;
	SSP_ReadWrite(this->peripheral, &xferConfig, SSP_TRANSFER_POLLING);
	return readByte;
}

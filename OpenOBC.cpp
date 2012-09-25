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

#include "OpenOBC.h"
#include "board.h"
#include "delay.h"

#include <lpc17xx_gpio.h>
#include <lpc17xx_pinsel.h>
#include <lpc17xx_adc.h>
#include <lpc17xx_clkpwr.h>
#include <lpc17xx_exti.h>

#include <cstdio>
#include <ctime>
#include <stdlib.h>
#include <algorithm>
#include <map>
#include <cmath>
#include "Timer.h"

using namespace std;

volatile uint32_t SysTickCnt;
OpenOBC* obcS;
Debug* debugS;
RTC* rtcS;

uint32_t rps;
uint32_t rpm;
uint32_t out0Bits;
uint32_t out1Bits;
bool doSleep = false;
bool go = false;
uint32_t speedPeriod;
uint32_t lastSpeedUpdateTime;

void callback()
{
	rps++;
}

extern "C" void Reset_Handler(void);
void speedHandler();
void runHandler();


OpenOBC::OpenOBC()
{
	GPIO_IntDisable(0, 0xffffffff, 0);
	GPIO_IntDisable(0, 0xffffffff, 1);
	GPIO_IntDisable(2, 0xffffffff, 0);
	GPIO_IntDisable(2, 0xffffffff, 1);

	obcS = this;
	
	SysTick_Config(SystemCoreClock/1000 - 1); //interrupt period 1ms
	debug = new Debug(DEBUG_TX_PORTNUM, DEBUG_TX_PINNUM, DEBUG_RX_PORTNUM, DEBUG_RX_PINNUM, DEBUG_BAUD, &interruptManager); debugS = debug;

	printf("-"); fflush(stdout); delay(250); printf("-"); fflush(stdout); delay(250); printf("-"); fflush(stdout); delay(250); printf(">\r\n"); delay(250);
	printf("clock speed: %i MHz\r\n", SystemCoreClock / 1000000);
	printf("stack: 0x%lx heap: 0x%lx free: %li\r\n", get_stack_top(), get_heap_end(), get_stack_top() - get_heap_end());

	//spi coniguration
	spi1 = new SPI(SPI1_MOSI_PORT, SPI1_MOSI_PIN, SPI1_MISO_PORT, SPI1_MISO_PIN, SPI1_SCK_PORT, SPI1_SCK_PIN, SPI1_CLOCKRATE);

	//lcd configuration
	lcdReset = new IO(LCD_RESET_PORT, LCD_RESET_PIN, false);
// 	IO lcdBias(LCD_BIASCLOCK_PORT, LCD_BIASCLOCK_PIN, true);
	lcdBiasClock = new PWM(LCD_BIASCLOCK_PORT, LCD_BIASCLOCK_PIN, .99);
	*lcdReset = true;
	IO* lcdSel = new IO(LCD_SELECT_PORT, LCD_SELECT_PIN, true);
	IO* lcdRefresh = new IO(LCD_REFRESH_PORT, LCD_REFRESH_PIN, false);
	IO* lcdUnk0 = new IO(LCD_UNK0_PORT, LCD_UNK0_PIN, true);
	IO* lcdUnk1 = new IO(LCD_UNK1_PORT, LCD_UNK1_PIN, false);
	lcd = new ObcLcd(*spi1, *lcdSel, *lcdRefresh, *lcdUnk0, *lcdUnk1);
	displayMode = DISPLAY_OPENOBC;

	//rtc configuration
	rtc = new RTC(); rtcS = rtc;
	RTC_TIME_Type settime;
	settime.YEAR = 2012;
	settime.MONTH = 9;
	settime.DOM = 20;
	settime.HOUR = 20;
	settime.MIN = 29;
	settime.SEC = 0;
// 	rtc->setTime(&settime);

	//ccm configuration
	Input* ccmData = new Input(CCM_DATA_PORT, CCM_DATA_PIN);
	IO* ccmClock = new IO(CCM_CLOCK_PORT, CCM_CLOCK_PIN);
	IO* ccmLatch = new IO(CCM_LATCH_PORT, CCM_LATCH_PIN);
	ccm = new CheckControlModule(*ccmData, *ccmClock, *ccmLatch);

	//fuel level configuration
	Input* fuelLevelInput = new Input(FUEL_LEVEL_PORT, FUEL_LEVEL_PIN);
	fuelLevel = new FuelLevel(*fuelLevelInput);

	//stalk button configuration
	stalkButton = new Input(STALK_BUTTON_PORT, STALK_BUTTON_PIN);

	//diagnostics interface configuration
	kline = new Uart(KLINE_TX_PORTNUM, KLINE_TX_PINNUM, KLINE_RX_PORTNUM, KLINE_RX_PINNUM, KLINE_BAUD, UART_PARITY_EVEN, &interruptManager);
	lline = new Uart(LLINE_TX_PORTNUM, LLINE_TX_PINNUM, LLINE_RX_PORTNUM, LLINE_RX_PINNUM, LLINE_BAUD, UART_PARITY_EVEN, &interruptManager);

	//ignition/run/on input configuration
	run = new Input(RUN_PORT, RUN_PIN);
	PINSEL_CFG_Type pincfg;
	pincfg.Funcnum = PINSEL_FUNC_1;
	pincfg.OpenDrain = PINSEL_PINMODE_NORMAL;
	pincfg.Pinmode = PINSEL_PINMODE_TRISTATE;
	pincfg.Portnum = RUN_PORT;
	pincfg.Pinnum = RUN_PIN;
	interruptManager.attach(IRQ_EINT1, &runHandler); //do this first...
	PINSEL_ConfigPin(&pincfg); //because this immediately triggers an unwanted interrupt
	EXTI_Init();
	EXTI_InitTypeDef EXTICfg;
	EXTICfg.EXTI_Line = EXTI_EINT1;
	EXTICfg.EXTI_Mode = EXTI_MODE_EDGE_SENSITIVE;
	EXTICfg.EXTI_polarity = EXTI_POLARITY_HIGH_ACTIVE_OR_RISING_EDGE;
	EXTI_ClearEXTIFlag(EXTI_EINT1);
	EXTI_Config(&EXTICfg);
	NVIC_SetPriorityGrouping(4);
	NVIC_SetPriority(EINT1_IRQn, 0);
	NVIC_EnableIRQ(EINT1_IRQn);

	//fuel consumption configuration
	fuelCons = new Capture(SPEED_PORT, SPEED_PIN, FUEL_CONS_PORT, FUEL_CONS_PIN, &interruptManager);
	fuelCons->attach(&callback);

	//speed configuration
	speed = new Input(SPEED_PORT, SPEED_PIN);
	speed->setPullup();
	interruptManager.attach(IRQ_EINT3, &speedHandler);
	GPIO_IntEnable(SPEED_PORT, (1<<SPEED_PIN), 1);
	NVIC_EnableIRQ(EINT3_IRQn);

	//sd card configuration
	sdcardDetect = new Input(SDCARD_DETECT_PORT, SDCARD_DETECT_PIN);
	sdcardDetect->setPullup();

	//keypad configuration
	IO* x0 = new IO(1, 20);
	IO* x1 = new IO(1, 21);
	IO* x2 = new IO(1, 22);
	IO* x3 = new IO(1, 25);
	IO* x4 = new IO(1, 27);
	Input* y0 = new Input(0, 0);
	Input* y1 = new Input(0, 1);
	Input* y2 = new Input(0, 10);
	Input* y3 = new Input(0, 11);
	keypad = new ObcKeypad(*x0, *x1, *x2, *x3, *x4, *y0, *y1, *y2, *y3, interruptManager);
	keypad->attach(BUTTON_KMMLS, this, &OpenOBC::setVoltage);
	keypad->attach(BUTTON_CONSUM, this, &OpenOBC::setConsum);
	keypad->attach(BUTTON_CODE, this, &OpenOBC::setTemp);
	keypad->attach(BUTTON_SPEED, this, &OpenOBC::setSpeed);
	keypad->attach(BUTTON_1, this, &OpenOBC::setOpenOBC);
	keypad->attach(BUTTON_CHECK, this, &OpenOBC::setCheck);
	keypad->attach(BUTTON_SET, this, &OpenOBC::buttonSet);
	keypad->attach(BUTTON_MEMO, this, &OpenOBC::buttonMemo);
	keypad->attach(BUTTON_DIST, this, &OpenOBC::buttonDist);
	keypad->attach(BUTTON_1000, this, &OpenOBC:: button1000);
	keypad->attach(BUTTON_100, this, &OpenOBC:: button100);

	//backlight configuration
	lcdLight = new IO(LCD_BACKLIGHT_PORT, LCD_BACKLIGHT_PIN, true);
	clockLight = new IO(CLOCK_BACKLIGHT_PORT, CLOCK_BACKLIGHT_PIN, true);
	keypadLight = new IO(KEYPAD_BACKLIGHT_PORT, KEYPAD_BACKLIGHT_PIN, true);
// 	lcdBacklight = new PWM(LCD_BACKLIGHT_PORT, LCD_BACKLIGHT_PIN, .2);
// 	clockBacklight = new PWM(CLOCK_BACKLIGHT_PORT, CLOCK_BACKLIGHT_PIN);

	//output driver configuration
	IO outRst(OUT_RESET_PORT, OUT_RESET_PIN, true);
	out0Cs = new IO(OUT0_CS_PORT, OUT0_CS_PIN, true);
	out1Cs = new IO(OUT1_CS_PORT, OUT1_CS_PIN, true);
	
	//adc configuration
	pincfg.Funcnum = 1;
	pincfg.OpenDrain = 0;
	pincfg.Pinmode = 0;
	pincfg.Portnum = 0;
	pincfg.Pinnum = 23;
	PINSEL_ConfigPin(&pincfg);
	pincfg.Portnum = 0;
	pincfg.Pinnum = 24;
	PINSEL_ConfigPin(&pincfg);
	pincfg.Portnum = 0;
	pincfg.Pinnum = 25;
	PINSEL_ConfigPin(&pincfg);
	
	ADC_Init(LPC_ADC, 200000);
	ADC_IntConfig(LPC_ADC,ADC_ADINTEN0,DISABLE);
// 	ADC_ChannelCmd(LPC_ADC,ADC_CHANNEL_0,ENABLE);
	ADC_IntConfig(LPC_ADC, ADC_ADINTEN2, DISABLE);
// 	ADC_ChannelCmd(LPC_ADC, ADC_CHANNEL_2, ENABLE);

	go = true;
}


void OpenOBC::mainloop()
{
	printf("stack: 0x%lx heap: 0x%lx free: %li\r\n", get_stack_top(), get_heap_end(), get_stack_top() - get_heap_end());
	printf("starting mainloop...\r\n");



// 	delay(1000);
// 	while(1)
// 	{
// 		if(get)
// 		{
// 			get = false;
// 			printf("frequency: %.0f duty cycle: %.2f speed: %.0f mph\r\n", speed->getFrequency(), speed->getDutyCycle(), speed->getFrequency() / 1.3 * .621);
// 		}
// 	}

	

	if(0)
	{
		//send zke4 inquiry
		lline->putc(0x00);
		lline->putc(0x04);
		lline->putc(0x00);
		lline->putc(0x04);
		
		lcd->printf("sent lline data");
		
		delay(1000);

		//verify transmission successfully reached data bus
		if(lline->readable())
		{
			if(lline->getc() == 0x00 && lline->getc() == 0x04 && lline->getc() == 0x00 && lline->getc() == 0x04)
			{
				lcd->printf("lline data ok"); //echo was received
				while(lline->readable())
					lline->getc(); //receive and discard the echo
			}
		}
		else
		{
			lcd->printf("no lline data"); //echo not received - interface failure
		}
		
		delay(1000);

		//print the zke4 reply to lcd
		if(kline->readable())
		{
			while(kline->readable())
			{
				lcd->printf("%x %x %x %x", kline->getc(), kline->getc(), kline->getc(), kline->getc());
				delay(1000);
			}
			lcd->printf("end of data");
		}
		else
			lcd->printf("no kline data"); //zke4 did not respond
		
		delay(2000);
	}

	while(1)
	{
		
		if(!*stalkButton)
			lcd->printfClock("  :D");
		else
			lcd->printfClock("%02i%02i", rtc->getHours(), rtc->getMinutes());

		float voltage;
		float resistance;
		float temperature;
		float speed;
		switch(displayMode)
		{
			case DISPLAY_VOLTAGE:
				ADC_ChannelCmd(LPC_ADC, ADC_CHANNEL_2, ENABLE);
				ADC_StartCmd(LPC_ADC, ADC_START_NOW);
				while (!(ADC_ChannelGetStatus(LPC_ADC, ADC_CHANNEL_2, ADC_DATA_DONE)));
				voltage = (float)ADC_ChannelGetData(LPC_ADC, ADC_CHANNEL_2) / 4095 * (2.990);
				ADC_ChannelCmd(LPC_ADC, ADC_CHANNEL_2, DISABLE);
				voltage = voltage / (2.2/(2.2+10.0));
				lcd->printf("%.2fV", voltage);
				break;

			case DISPLAY_SPEED:
				if(SysTickCnt - lastSpeedUpdateTime > 2000)
					speed = 0;
				else
					speed = (float)1000000 / speedPeriod / 4712 * 3600 * .621;
				lcd->printf("%3.1f mph", speed);
				break;
				
			case DISPLAY_TEMP:
				ADC_ChannelCmd(LPC_ADC, ADC_CHANNEL_0, ENABLE);
				ADC_StartCmd(LPC_ADC,ADC_START_NOW);
				while(!(ADC_ChannelGetStatus(LPC_ADC,ADC_CHANNEL_0, ADC_DATA_DONE)));
				voltage = (float)ADC_ChannelGetData(LPC_ADC, ADC_CHANNEL_0) / 4095 * (2.990);
				ADC_ChannelCmd(LPC_ADC, ADC_CHANNEL_0, DISABLE);
				resistance = (10000 * voltage) / (3.3 - voltage);
				temperature = 1.0 / ((1.0 / 298.15) + (1.0/3950) * log(resistance / 4700)) - 273.15;
				lcd->printf("%.1fC  %.1fF", temperature, temperature * 1.78 + 32);
				printf("V: %.3f R: %.0f T: %.2f\r\n", voltage, resistance, temperature);
				break;
				
			case DISPLAY_CONSUM:
				lcd->printf("%.6f rpm: %4i", fuelCons->getOnTime(), rpm);
				break;
				
			case DISPLAY_OPENOBC:
				lcd->printf("openOBC");
				break;
				
			case DISPLAY_CHECK:
				lcd->printf("CCM byte: %02x", ccm->getStatus());
				break;

			case DISPLAY_FREEMEM:
				lcd->printf("free memory: %i", get_stack_top() - get_heap_end());
				break;

			case DISPLAY_FUEL_LEVEL:
// 				lcd->printf("%.1f litres", (float)fuelLevel->getLitres() / 10);
				lcd->printf("%.1f gallons", (float)fuelLevel->getLitres() / 10 * 0.264172);
				break;

			case DISPLAY_OUTPUTS:
				lcd->printf("outputs: 0x%02x 0x%02x", out0Bits, out1Bits);
				break;

			default:
				lcd->printf("invalid displayMode");
				break;
				
		}

		delay(50);

		if(doSleep)
		{
			while(doSleep)
				sleep();
			wake();
		}
	}

}

void OpenOBC::sleep()
{
	*obcS->lcdLight = false;
	*obcS->clockLight = false;
	*obcS->keypadLight = false;

	NVIC_DisableIRQ(EINT3_IRQn);
	GPIO_IntDisable(0, 0xffffffff, 0);
	GPIO_IntDisable(0, 0xffffffff, 1);
	GPIO_IntDisable(2, 0xffffffff, 0);
	GPIO_IntDisable(2, 0xffffffff, 1);

	/*---------- Disable and disconnect the main PLL0 before enter into Deep-Sleep
	 * or Power-Down mode <according to errata.lpc1768-16.March.2010> ------------
	 */
	LPC_SC->PLL0CON &= ~(1<<1); /* Disconnect the main PLL (PLL0) */
	LPC_SC->PLL0FEED = 0xAA; /* Feed */
	LPC_SC->PLL0FEED = 0x55; /* Feed */
	while ((LPC_SC->PLL0STAT & (1<<25)) != 0x00); /* Wait for main PLL (PLL0) to disconnect */
	LPC_SC->PLL0CON &= ~(1<<0); /* Turn off the main PLL (PLL0) */
	LPC_SC->PLL0FEED = 0xAA; /* Feed */
	LPC_SC->PLL0FEED = 0x55; /* Feed */
	while ((LPC_SC->PLL0STAT & (1<<24)) != 0x00); /* Wait for main PLL (PLL0) to shut down */
	
	/*------------Then enter into PowerDown mode ----------------------------------*/
// 	CLKPWR_Sleep();
// 	CLKPWR_DeepSleep();
	CLKPWR_PowerDown();
// 	CLKPWR_DeepPowerDown();

}

void OpenOBC::wake()
{
// 	SystemInit();
	Reset_Handler();
	*obcS->lcdLight = true;
	*obcS->clockLight = true;
	*obcS->keypadLight = true;
}

void OpenOBC::setConsum()
{
	displayMode = DISPLAY_CONSUM;
}

void OpenOBC::setSpeed()
{
	displayMode = DISPLAY_SPEED;
}

void OpenOBC::setTemp()
{
	displayMode = DISPLAY_TEMP;
}

void OpenOBC::setVoltage()
{
	displayMode = DISPLAY_VOLTAGE;
}

void OpenOBC::setOpenOBC()
{
	displayMode = DISPLAY_OPENOBC;
}

void OpenOBC::setCheck()
{
	displayMode = DISPLAY_CHECK;
}

void OpenOBC::buttonDist()
{
	displayMode = DISPLAY_FUEL_LEVEL;
}

void OpenOBC::buttonSet()
{
	
}

void OpenOBC::button1000()
{
	if(displayMode != DISPLAY_OUTPUTS)
	{
		displayMode = DISPLAY_OUTPUTS;
		return;
	}
	out0Bits *= 2;
	if(out0Bits == 0)
		out0Bits = (1<<0);
	if(out0Bits > (1<<7))
		out0Bits = 0;
	
	*out0Cs = false;
	obcS->spi1->putc(out0Bits);
	*out0Cs = true;
}

void OpenOBC::button100()
{
	if(displayMode != DISPLAY_OUTPUTS)
	{
		displayMode = DISPLAY_OUTPUTS;
		return;
	}
	out1Bits *= 2;
	if(out1Bits == 0)
		out1Bits = (1<<0);
	if(out1Bits > (1<<7))
		out1Bits = 0;
	
	*out1Cs = false;
	obcS->spi1->putc(out1Bits);
	*out1Cs = true;
}

void OpenOBC::buttonMemo()
{
	displayMode = DISPLAY_FREEMEM;
}

void runHandler()
{
	EXTI_ClearEXTIFlag(EXTI_EINT1);
	doSleep = false;
}

void speedHandler()
{
	static Timer periodTimer;
	if(GPIO_GetIntStatus(SPEED_PORT, SPEED_PIN, 1))
	{
		speedPeriod = periodTimer.read_us();
		periodTimer.start();
		GPIO_ClearInt(SPEED_PORT, (1<<SPEED_PIN));
		lastSpeedUpdateTime = SysTickCnt;
	}
}

extern "C" void allocate_failed()
{
	while (1);
}

extern "C" void check_failed(uint8_t* file, uint32_t line)
{
	printf("CHECK FAILED (%s:%i)\r\n", file, line);
	while (1);
}

extern "C" void HardFault_Handler()
{
	while(1);
}

extern "C" void SysTick_Handler()
{
	SysTickCnt++;

// 	static bool blinky;
// 	if(SysTickCnt % 500 == 0 && go)
// 	{
// 		if(blinky = !blinky)
// 		{
// 			out0Cs = false;
// 			obcS->spi1->putc(0x04);
// 			out0Cs = true;
// 		}
// 		else
// 		{
// 			out0Cs = false;
// 			obcS->spi1->putc(0x00);
// 			out0Cs = true;
// 		}
// 	}

	if(go)
	{
		if(SysTickCnt % 1000 == 0)
		{
			rpm = rps * 60;
			rps = 0;
		}

		if(*obcS->run)
			doSleep = false;
		else
			doSleep = true;
	}
}

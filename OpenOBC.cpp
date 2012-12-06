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
#include "Timer.h"
#include "debugpretty.h"

#include <lpc17xx_gpio.h>
#include <lpc17xx_pinsel.h>
#include <lpc17xx_clkpwr.h>
#include <lpc17xx_exti.h>

#include <cstdio>
#include <cmath>
#include <stdlib.h>
#include <Bus.h>

volatile uint32_t SysTickCnt;
OpenOBC* obcS;
Debug* debugS;
RTC* rtcS;

uint32_t out0Bits;
uint32_t out1Bits;
bool doSleep = false;
bool go = false;
IO* idle;
IO* isr;
bool doQuery = false;
bool doUnlock = false;
bool doLock = false;
bool doSnoop = false;

extern "C" void Reset_Handler(void);

void runHandler();

void OpenOBC::printDS2Packet()
{
	DS2Packet* packet;
	while(1)
	{
		if((packet = diag->read(DS2_K)) != NULL)
		{
			fprintf(stderr, "received packet on K: ");
			packet->printPacket(stderr);
			fprintf(stderr, "\r");
			delete packet;
		}
		else if((packet = diag->read(DS2_L)) != NULL)
		{
			fprintf(stderr, "received packet on L: ");
			packet->printPacket(stderr);
			fprintf(stderr, "\r");
			delete packet;
		}
		else
			break;
	}
}

OpenOBC::OpenOBC()
{
	GPIO_IntDisable(0, 0xffffffff, 0);
	GPIO_IntDisable(0, 0xffffffff, 1);
	GPIO_IntDisable(2, 0xffffffff, 0);
	GPIO_IntDisable(2, 0xffffffff, 1);

	//enable bus, usage, and memmanage faults
	SCB->SHCSR |= (1<<18) | (1<<17) | (1<<16);
	
	idle = new IO(2, 0, true);
	isr = new IO(2, 1, true);
	obcS = this;
	
	SysTick_Config(SystemCoreClock/1000 - 1); //interrupt period 1ms
	debug = new Debug(DEBUG_TX_PORTNUM, DEBUG_TX_PINNUM, DEBUG_RX_PORTNUM, DEBUG_RX_PINNUM, DEBUG_BAUD, &interruptManager); debugS = debug;

	printf("-"); fflush(stdout); delay(250); printf("-"); fflush(stdout); delay(250); printf("-"); fflush(stdout); delay(250); printf(">\r\n"); delay(250);
	printf("clock speed: %i MHz\r\n", SystemCoreClock / 1000000);
	printf("stack: 0x%lx heap: 0x%lx free: %li\r\n", get_stack_top(), get_heap_end(), get_stack_top() - get_heap_end());

	//spi coniguration
	spi1 = new SPI(SPI1_MOSI_PORT, SPI1_MOSI_PIN, SPI1_MISO_PORT, SPI1_MISO_PIN, SPI1_SCK_PORT, SPI1_SCK_PIN, SPI1_CLOCKRATE);
	
	//config file configuration
	IO* sdcardCs = new IO(SDCARD_CS_PORT, SDCARD_CS_PIN);
	sd = new SDFS(*spi1, *sdcardCs);
	sd->mount("sd");
	config = new ConfigFile("/sd/openobc.cfg");
	//parse the config file; create one with default parameters if it doesn't exist
	if(config->readConfig() < 0)
	{
		//default config file parameters
		config->setValueByName("DefaultDisplayMode", "DISPLAY_OPENOBC");
		config->setValueByName("VoltageReferenceCalibration", "0.00");
		config->setValueByName("MeasurementSystem", "METRIC");
		
		int32_t result = config->writeConfig();
		if(result > 0)
		{
			fprintf(stderr, "wrote default config file to %s\r\n", config->getFilename().c_str());
		}
	}

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

	//default display mode configuration
	std::string defaultDisplayModeString = config->getValueByName("DefaultDisplayMode");
	if(defaultDisplayModeString == "DISPLAY_CHECK")
		displayMode = DISPLAY_CHECK;
	else if(defaultDisplayModeString == "DISPLAY_CONSUM1")
		displayMode = DISPLAY_CONSUM1;
	else if(defaultDisplayModeString == "DISPLAY_CONSUM2")
		displayMode = DISPLAY_CONSUM2;
	else if(defaultDisplayModeString == "DISPLAY_CONSUM3")
		displayMode = DISPLAY_CONSUM3;
	else if(defaultDisplayModeString == "DISPLAY_FREEMEM")
		displayMode = DISPLAY_FREEMEM;
	else if(defaultDisplayModeString == "DISPLAY_FUEL_LEVEL")
		displayMode = DISPLAY_FUEL_LEVEL;
	else if(defaultDisplayModeString == "DISPLAY_OPENOBC")
		displayMode = DISPLAY_OPENOBC;
	else if(defaultDisplayModeString == "DISPLAY_OUTPUTS")
		displayMode = DISPLAY_OUTPUTS;
	else if(defaultDisplayModeString == "DISPLAY_SPEED")
		displayMode = DISPLAY_SPEED;
	else if(defaultDisplayModeString == "DISPLAY_TEMP")
		displayMode = DISPLAY_TEMP;
	else if(defaultDisplayModeString == "DISPLAY_VOLTAGE")
		displayMode = DISPLAY_VOLTAGE;
	else
		displayMode = DISPLAY_OPENOBC;
	if(config->getValueByName("MeasurementSystem") == "METRIC")
		useMetricSystem = true;
	else if(config->getValueByName("MeasurementSystem") == "IMPERIAL")
		useMetricSystem = false;
	else
		useMetricSystem = false;

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
	fuelLevel = new FuelLevel(*fuelLevelInput, interruptManager);

	//stalk button configuration
	stalkButton = new Input(STALK_BUTTON_PORT, STALK_BUTTON_PIN);

	//diagnostics interface configuration
	kline = new Uart(KLINE_TX_PORTNUM, KLINE_TX_PINNUM, KLINE_RX_PORTNUM, KLINE_RX_PINNUM, KLINE_BAUD, UART_PARITY_EVEN, &interruptManager);
	lline = new Uart(LLINE_TX_PORTNUM, LLINE_TX_PINNUM, LLINE_RX_PORTNUM, LLINE_RX_PINNUM, LLINE_BAUD, UART_PARITY_EVEN, &interruptManager);
	DS2Bus* k = new Bus(*kline);
	DS2Bus* l = new Bus(*lline);
	diag = new DS2(*k, *l);

// 	while(1)
// 	{
// 		int length = (rand() % 15) + 1;
// 		uint8_t* data = new uint8_t[length];
// 		uint32_t r = rand();
// 		for(int i = 0; i < length; i++)
// 			data[i] = r >> (i*2);
// 		DS2Packet packet(0x55, data, length, DS2_16BIT);
// 		delete[] data;
// 		k->write(packet.getRawPacket(), packet.getPacketLength());
// // 		diag->query(packet, DS2_K);
// 		delay(200);
// 	}

	zke = new E36ZKE4(*diag);
	srs = new E36SRS(*diag);
	ihkr = new E36IHKR(*diag);
	kombi = new E36Kombi(*diag);
	mk4 = new E36MK4(*diag);
	
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
	Input* fuelConsInput = new Input(FUEL_CONS_PORT, FUEL_CONS_PIN);
	fuelConsInput->setPullup();
	fuelCons = new FuelConsumption(*fuelConsInput, interruptManager);

	//speed configuration
	Input* speedPin = new Input(SPEED_PORT, SPEED_PIN);
	speedPin->setPullup();
	speed = new SpeedInput(*speedPin, interruptManager);

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
	keypad->attach(BUTTON_1000, this, &OpenOBC::button1000);
	keypad->attach(BUTTON_100, this, &OpenOBC::button100);
	keypad->attach(BUTTON_1, this, &OpenOBC::button1);
	keypad->attach(BUTTON_CONSUM, this, &OpenOBC::buttonConsum);
	keypad->attach(BUTTON_RANGE, this, &OpenOBC::buttonRange);
	keypad->attach(BUTTON_CODE, this, &OpenOBC::buttonTemp);
	keypad->attach(BUTTON_SPEED, this, &OpenOBC::buttonSpeed);
	keypad->attach(BUTTON_DIST, this, &OpenOBC::buttonDist);
	keypad->attach(BUTTON_TIMER, this, &OpenOBC::buttonTimer);
	keypad->attach(BUTTON_CHECK, this, &OpenOBC::buttonCheck);
	keypad->attach(BUTTON_KMMLS, this, &OpenOBC::buttonKMMLS);
	keypad->attach(BUTTON_CLOCK, this, &OpenOBC::buttonClock);
	keypad->attach(BUTTON_DATE, this, &OpenOBC::buttonDate);
	keypad->attach(BUTTON_MEMO, this, &OpenOBC::buttonMemo);
	keypad->attach(BUTTON_SET, this, &OpenOBC::buttonSet);

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
	
	//analog input configuration
	batteryVoltage = new AnalogIn(BATTERY_VOLTAGE_PORT, BATTERY_VOLTAGE_PIN, REFERENCE_VOLTAGE + atof(config->getValueByName("VoltageReferenceCalibration").c_str()), (10 + 2.2) / 2.2 * REFERENCE_VOLTAGE);
	temperature = new AnalogIn(EXT_TEMP_PORT,EXT_TEMP_PIN, REFERENCE_VOLTAGE + atof(config->getValueByName("VoltageReferenceCalibration").c_str()));

	printf("openOBC firmware version: %s\r\n", GIT_VERSION);
	lcd->printf("openOBC %s", GIT_VERSION);
	delay(3000);
	
	go = true;
}


void OpenOBC::mainloop()
{
	printf("stack: 0x%lx heap: 0x%lx free: %li\r\n", get_stack_top(), get_heap_end(), get_stack_top() - get_heap_end());
	printf("starting mainloop...\r\n");

// 	doSnoop = true;
// 	diag->attach(this, &OpenOBC::printDS2Packet);
	
	while(1)
	{
		if(!*stalkButton)
			lcd->printfClock("  :D");
		else
			lcd->printfClock("%02i%02i", rtc->getHours(), rtc->getMinutes());

		ccm->task();
		diag->task();

		switch(displayMode)
		{
			case DISPLAY_VOLTAGE:
			{
				lcd->printf("%.2fV", batteryVoltage->read());
				break;
			}
			case DISPLAY_SPEED:
			{
				if(useMetricSystem)
					lcd->printf("%3.1f km/h", speed->getSpeed());
				else
					lcd->printf("%3.1f mph", speed->getSpeed() * 0.621371);
				break;
			}
			case DISPLAY_TEMP:
			{
				float voltage = temperature->read();
				float resistance = (10000 * voltage) / (3.3 - voltage);
				float temperature = 1.0 / ((1.0 / 298.15) + (1.0/3950) * log(resistance / 4700)) - 273.15f;
				lcd->printf("%.1fC  %.1fF", temperature, temperature * 1.78 + 32);
				break;
			}
			case DISPLAY_CONSUM1:
			{
				float litresPerHour = 0.2449 * 6 * 60 * fuelCons->getDutyCycle();
				float gallonsPerHour = litresPerHour / 3.78514;
				float kilometresPerHour = speed->getSpeed();
				float milesPerHour = kilometresPerHour * 0.621371;
				if(useMetricSystem)
					lcd->printf("%i L/100km", litresPerHour / (100 * kilometresPerHour));
				else
					lcd->printf("%.1f mpg", milesPerHour / gallonsPerHour);
				break;
			}
			case DISPLAY_CONSUM2:
			{
				float litresPerMinute = 0.2449 * 6 * fuelCons->getDutyCycle();
				float gallonsPerMinute = litresPerMinute / 3.78514;
				if(useMetricSystem)
					lcd->printf("%.3f L/min", litresPerMinute);
				else
					lcd->printf("%.3f gal/hour", gallonsPerMinute * 60);
				break;
			}
			case DISPLAY_CONSUM3:
			{
				lcd->printf("%2.1f%% rpm: %4.0f", fuelCons->getDutyCycle() * 100, fuelCons->getRpm());
				break;
			}
			case DISPLAY_OPENOBC:
			{
				lcd->printf("openOBC");
				break;
			}	
			case DISPLAY_CHECK:
			{
				lcd->printf("CCM byte: %02x", ccm->getRawByte());
				break;
			}
			case DISPLAY_FREEMEM:
			{
				lcd->printf("free memory: %i", get_stack_top() - get_heap_end());
				break;
			}
			case DISPLAY_FUEL_LEVEL:
			{
				if(useMetricSystem)
					lcd->printf("%.1f litres", fuelLevel->getLitres());
				else
					lcd->printf("%.2f gallons", fuelLevel->getGallons());
				break;
			}
			case DISPLAY_OUTPUTS:
			{
				lcd->printf("outputs: 0x%02x 0x%02x", out0Bits, out1Bits);
				break;
			}
			default:
			{
				lcd->printf("invalid displayMode");
				break;
			}	
		}
		
		*out0Cs = false;
		spi1->readWrite(out0Bits);
		*out0Cs = true;
		*out1Cs = false;
		spi1->readWrite(out1Bits);
		*out1Cs = true;

		if(doQuery)
		{
			printf("querying...\r\n");
			zke->query();
			srs->query();
			ihkr->query();
			kombi->query();
			mk4->query();
			printf("finished querying\r\n");
			doQuery = false;
		}
		if(doUnlock)
		{
			printf("doing unlock\r\n");
			zke->unlock();
			printf("done\r\n");
			doUnlock = false;
		}
		if(doLock)
		{
			printf("doing lock\r\n");
			zke->lock();
			printf("done\r\n");
			doLock = false;
		}

		delay(200);

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
	*obcS->clockLight = true;
	delay(160); //80 = 2 seconds at 4MHz
// 	SystemInit();
	*obcS->lcdLight = true;
	*obcS->keypadLight = true;
	__set_MSP(0x10008000);
	Reset_Handler();
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
}

void OpenOBC::button1()
{
	displayMode = DISPLAY_OPENOBC;
}

void OpenOBC::buttonConsum()
{
	if(displayMode == DISPLAY_CONSUM1)
		displayMode = DISPLAY_CONSUM2;
	else if(displayMode == DISPLAY_CONSUM2)
		displayMode = DISPLAY_CONSUM3;
	else if(displayMode == DISPLAY_CONSUM3)
		displayMode = DISPLAY_CONSUM1;
	else
		displayMode = DISPLAY_CONSUM1;
}

void OpenOBC::buttonRange()
{
	displayMode = DISPLAY_FUEL_LEVEL;
}

void OpenOBC::buttonTemp()
{
	displayMode = DISPLAY_TEMP;
}

void OpenOBC::buttonSpeed()
{
	displayMode = DISPLAY_SPEED;
}

void OpenOBC::buttonDist()
{
	displayMode = DISPLAY_VOLTAGE;
}

void OpenOBC::buttonTimer()
{
	doQuery = true;
}

void OpenOBC::buttonCheck()
{
	displayMode = DISPLAY_CHECK;
}

void OpenOBC::buttonKMMLS()
{
	useMetricSystem = !useMetricSystem;
}

void OpenOBC::buttonClock()
{
	doUnlock = true;
}

void OpenOBC::buttonDate()
{
	doLock = true;
}

void OpenOBC::buttonMemo()
{
	displayMode = DISPLAY_FREEMEM;
}

void OpenOBC::buttonSet()
{
	if(doSnoop)
	{
		printf("not snooping\r\n");
		diag->detach(this, &OpenOBC::printDS2Packet);
		doSnoop = false;
	}
	else
	{
		printf("snooping...\r\n");
		diag->attach(this, &OpenOBC::printDS2Packet);
		doSnoop = true;
	}
}

void runHandler()
{
	EXTI_ClearEXTIFlag(EXTI_EINT1);
	doSleep = false;
}

extern "C" void allocate_failed()
{
	fprintf(stderr, "%cALLOCATE FAILED\r\n", 0x7);
	throw;
}

extern "C" void check_failed(uint8_t* file, uint32_t line)
{
	printf("CHECK FAILED (%s:%i)\r\n", file, line);
	throw;
}

extern "C" void MemManage_Handler()
{
	fprintf(stderr, "%cMEMMANAGE FAULT\r\n", 0x7);
	__asm("TST LR, #4\n"
	"ITE EQ\n"
	"MRSEQ R0, MSP\n"
	"MRSNE R0, PSP\n"
	"B hard_fault_handler_c\n");
}

extern "C" void BusFault_Handler()
{
// 	printf("%cBUS FAULT\r\n", 0x7);
	__asm("TST LR, #4\n"
	"ITE EQ\n"
	"MRSEQ R0, MSP\n"
	"MRSNE R0, PSP\n"
	"B hard_fault_handler_c\n");
}

extern "C" void UsageFault_Handler()
{
	printf("%cUSAGE FAULT\r\n", 0x7);
	__asm("TST LR, #4\n"
	"ITE EQ\n"
	"MRSEQ R0, MSP\n"
	"MRSNE R0, PSP\n"
	"B hard_fault_handler_c\n");
}

extern "C" void HardFault_Handler()
{
	__asm("TST LR, #4\n"
	"ITE EQ\n"
	"MRSEQ R0, MSP\n"
	"MRSNE R0, PSP\n"
	"B hard_fault_handler_c\n");
}

extern "C" void hard_fault_handler_c(unsigned int * hardfault_args)
{
	printf("%cHARD FAULT\r\n", 0x7);
	
	unsigned int stacked_r0;
	unsigned int stacked_r1;
	unsigned int stacked_r2;
	unsigned int stacked_r3;
	unsigned int stacked_r12;
	unsigned int stacked_lr;
	unsigned int stacked_pc;
	unsigned int stacked_psr;
	
	stacked_r0 = ((unsigned long) hardfault_args[0]);
	stacked_r1 = ((unsigned long) hardfault_args[1]);
	stacked_r2 = ((unsigned long) hardfault_args[2]);
	stacked_r3 = ((unsigned long) hardfault_args[3]);
	
	stacked_r12 = ((unsigned long) hardfault_args[4]);
	stacked_lr = ((unsigned long) hardfault_args[5]);
	stacked_pc = ((unsigned long) hardfault_args[6]);
	stacked_psr = ((unsigned long) hardfault_args[7]);

	
	printf ("\r\n[Hard fault handler - all numbers in hex]\r\n");
	printf ("R0 = %x\r\n", stacked_r0);
	printf ("R1 = %x\r\n", stacked_r1);
	printf ("R2 = %x\r\n", stacked_r2);
	printf ("R3 = %x\r\n", stacked_r3);
	printf ("R12 = %x\r\n", stacked_r12);
	printf ("LR [R14] = %x  subroutine call return address\r\n", stacked_lr);
	printf ("PC [R15] = %x  program counter\r\n", stacked_pc);
	printf ("PSR = %x\r\n", stacked_psr);
	printf ("BFAR = %x\r\n", (*((volatile unsigned long *)(0xE000ED38))));
	printf ("CFSR = %x\r\n", (*((volatile unsigned long *)(0xE000ED28))));
	printf ("HFSR = %x\r\n", (*((volatile unsigned long *)(0xE000ED2C))));
	printf ("DFSR = %x\r\n", (*((volatile unsigned long *)(0xE000ED30))));
	printf ("AFSR = %x\r\n", (*((volatile unsigned long *)(0xE000ED3C))));
	printf ("SCB_SHCSR = %x\r\n", SCB->SHCSR);

	
	throw;
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
		if(*obcS->run)
			doSleep = false;
		else
			doSleep = true;
	}
}

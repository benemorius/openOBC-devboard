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
int ctemp = 0x32;
int fuelc = 0x00;
int seti = 5;
int setvalue = 0;

uint8_t reply8[] = {0x0d, 0x00, 0x13, 0x08, 0x03, 0x05, 0x17, 0x90, 0xb5, 0x5d, 0xff, 0x01, 0xec, 0xff, 0xff, 0xff, 0xff, 0x7f, 0x12};
const uint8_t coolant_temp_table[256] = {200, 199, 196, 192, 189, 185, 182, 179, 175, 172, 169, 165, 162, 158, 155, 152, 148, 145, 142, 138, 135, 131, 
130, 128, 126, 125, 123, 121, 120, 118, 116, 115, 114, 113, 112, 111, 110, 108, 107, 106, 104, 103, 102, 101, 101, 99, 99, 98, 97, 97, 96, 95, 94, 93, 93, 
92, 91, 90, 89, 88, 88, 87, 86, 85, 85, 84, 84, 83, 83, 82, 81, 81, 80, 80, 79, 79, 78, 77, 76, 76, 75, 75, 74, 74, 73, 72, 72, 71, 71, 71, 70, 70, 69, 69, 69, 68, 
67, 67, 67, 66, 66, 65, 65, 65, 64, 63, 63, 62, 62, 62, 61, 61, 60, 60, 60, 59, 58, 58, 57, 57, 57, 56, 55, 54, 54, 53, 53, 52, 52, 51, 51, 50, 50, 49, 49, 48, 48, 
47, 47, 46, 46, 45, 44, 44, 43, 43, 42, 42, 41, 40, 40, 40, 39, 39, 38, 38, 37, 37, 36, 35, 35, 34, 34, 33, 33, 32, 31, 31, 30, 30, 30, 29, 29, 28, 28, 27, 26, 26, 
25, 25, 24, 24, 23, 22, 22, 21, 21, 20, 20, 20, 19, 19, 18, 17, 17, 16, 16, 15, 15, 15, 15, 15, 14, 14, 14, 13, 13, 13, 13, 13, 12, 12, 12, 12, 12, 11, 11, 11, 11, 
11, 10, 10, 10, 10, 9, 9, 8, 8, 8, 8, 7, 7, 7, 7, 6, 6, 6, 6, 5, 5, 4, 4, 4, 3, 3, 3, 3, 2, 2, 2, 2, 1, 1, 1, 1, 0};

extern "C" void Reset_Handler(void);

void runHandler(bool isLast = false);

void OpenOBC::printDS2Packet(bool isLast)
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
			
			//check packet and send reply
			if((packet->getType() == DS2_16BIT) && (packet->getDataLength() == 1))
			{
				char data = packet->getData()[0];
				const uint8_t reply0[] = {0x0d, 0x00, 0x0b, 0x00, 0x03, 0x37, 0xbe, 0x25, 0x02, 0x94, 0x3f};
// 				reply8[5] = setvalue; //set 58g high time (raw value)
// 				reply8[6] = setvalue; //set 58g low time (raw value)
// 				reply8[7] = setvalue; //set clamp 30 voltage
// 				reply8[8] = setvalue; //set coolant temperature
// 				reply8[9] = setvalue; //set fuel capacity
// 				reply8[10] = setvalue; //set ???
// 				reply8[11] = setvalue; //set ec signal
// 				reply8[12] = setvalue; //set ec signal
// 				reply8[13] = setvalue; //set rpm
// 				reply8[14] = setvalue; //set rpm
// 				reply8[15] = setvalue; //set speed
// 				reply8[16] = setvalue; //set speed
// 				reply8[17] = setvalue; //set digital values

				reply8[seti] = setvalue;
				
				if(data == 0x00)
				{
					DS2Packet* reply = new DS2Packet(reply0, 11, DS2_16BIT);
					diag->write(*reply, DS2_K);
					delete reply;
					DEBUG("replied to 0\r\n");
				}
				else if(data == 0x08)
				{
					DS2Packet* reply = new DS2Packet(reply8, 0x17, DS2_16BIT);
					diag->write(*reply, DS2_K);
					delete reply;
					DEBUG("replied to 8\r\n");
				}
			}
			else if(((packet->getType() == DS2_16BIT) && (packet->getDataLength() == 5)))
			{
				uint8_t* data = packet->getData();
				const uint8_t replyc[] = {0x0d, 0x00, 0x07, 0x0c, 0x97, 0x02, 0x93};
				const uint8_t replyc1[] = {0x0d, 0x00, 0x07, 0x0c, 0x73, 0x58, 0x2d};
				if(data[0] == 0x0c && data[4] == 0x00)
				{
					DS2Packet* reply = new DS2Packet(replyc, 7, DS2_16BIT);
					diag->write(*reply, DS2_K);
					delete reply;
					DEBUG("replied to c\r\n");
				}
				else if(data[0] == 0x0c && data[4] == 0x10)
				{
					DS2Packet* reply = new DS2Packet(replyc1, 7, DS2_16BIT);
					diag->write(*reply, DS2_K);
					delete reply;
					DEBUG("replied to c1\r\n");
				}
			}
			
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
	
	callback = new Callback();
// 	callback->addCallback(this, &OpenOBC::buttonMemo, 10000);
	
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
		int32_t result = config->writeConfig();
		if(result >= 0)
		{
			//default config file parameters
			config->setValueByName("DefaultDisplayMode", "DISPLAY_OPENOBC");
			config->setValueByName("VoltageReferenceCalibration", "0.00");
			config->setValueByName("MeasurementSystem", "METRIC");
			fprintf(stderr, "wrote default config file to %s\r\n", config->getFilename().c_str());
		}
		else
		{
			DEBUG("couldn't open file for writing\r\n");
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
	else if(defaultDisplayModeString == "DISPLAY_CONSUM4")
		displayMode = DISPLAY_CONSUM4;
	else if(defaultDisplayModeString == "DISPLAY_FREEMEM")
		displayMode = DISPLAY_FREEMEM;
	else if(defaultDisplayModeString == "DISPLAY_RANGE1")
		displayMode = DISPLAY_RANGE1;
	else if(defaultDisplayModeString == "DISPLAY_RANGE2")
		displayMode = DISPLAY_RANGE2;
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
	clockDisplayMode = CLOCKDISPLAY_CLOCK;

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
	keypad->attach(BUTTON_10, this, &OpenOBC::button10);
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
	keypad->attach(BUTTON_MEMO, this, &OpenOBC::buttonSet);
	keypad->attach(BUTTON_SET, this, &OpenOBC::buttonSet);

	//backlight configuration
	lcdLight = new IO(LCD_BACKLIGHT_PORT, LCD_BACKLIGHT_PIN, true);
	clockLight = new IO(CLOCK_BACKLIGHT_PORT, CLOCK_BACKLIGHT_PIN, true);
	keypadLight = new IO(KEYPAD_BACKLIGHT_PORT, KEYPAD_BACKLIGHT_PIN, true);
// 	lcdBacklight = new PWM(LCD_BACKLIGHT_PORT, LCD_BACKLIGHT_PIN, .2);
// 	clockBacklight = new PWM(CLOCK_BACKLIGHT_PORT, CLOCK_BACKLIGHT_PIN);

	//output driver configuration
	IO outRst(OUT_RESET_PORT, OUT_RESET_PIN, true);
	IO* out0Cs = new IO(OUT0_CS_PORT, OUT0_CS_PIN, true);
	IO* out1Cs = new IO(OUT1_CS_PORT, OUT1_CS_PIN, true);
	out0 = new MAX4896(*spi1, *out0Cs);
	out1 = new MAX4896(*spi1, *out1Cs);
	codeLed = new MAX4896Pin(*out0, (1<<2));
	limitLed = new MAX4896Pin(*out0, (1<<3));
	timerLed = new MAX4896Pin(*out0, (1<<4));
	ccmLight = new MAX4896Pin(*out0, (1<<5));
	ews = new MAX4896Pin(*out1, (1<<6));

	limitLed->on();
	
	//analog input configuration
	batteryVoltage = new AnalogIn(BATTERY_VOLTAGE_PORT, BATTERY_VOLTAGE_PIN, REFERENCE_VOLTAGE + atof(config->getValueByName("VoltageReferenceCalibration").c_str()), (10 + 2.2) / 2.2 * REFERENCE_VOLTAGE);
	temperature = new AnalogIn(EXT_TEMP_PORT,EXT_TEMP_PIN, REFERENCE_VOLTAGE + atof(config->getValueByName("VoltageReferenceCalibration").c_str()));
	
	averageFuelConsumptionSeconds = 0;
	averageLitresPer100km = 0;

	printf("openOBC firmware version: %s\r\n", GIT_VERSION);
	lcd->printf("openOBC %s", GIT_VERSION);
	delay(1500);
	
	go = true;
}

void uarthandler(bool isLast)
{
	while(debugS->readable())
	{
		static int esc;
		char c = debugS->get();
		DEBUG("0x%x\r\n", c);
		if(esc == 0)
		{
			if(c == 0x1b)
				esc = 1;
			else if(c == 'u')
				++ctemp;
			else if(c == 'd')
				--ctemp;
		}
		else if(esc == 1)
		{
			if(esc == 1 && c == 0x5b)
				esc = 2;
			else
				esc = 0;
		}
		else if(esc == 2)
		{
			esc = 0;
			if(c == 0x41)
				++setvalue;
			else if(c == 0x42)
				--setvalue;
			else if(c == 0x43)
			{
				++seti;
				if(seti < 5)
					seti = 5;
				if(seti > 17)
					seti = 17;
				setvalue = reply8[seti];
			}
			else if(c == 0x44)
			{
				--seti;
				if(seti < 5)
					seti = 5;
				if(seti > 17)
					seti = 17;
				setvalue = reply8[seti];
			}

		}
		else
			esc = 0;
	}
}

void OpenOBC::mainloop()
{
	debug->attach(&uarthandler);
	
	printf("stack: 0x%lx heap: 0x%lx free: %li\r\n", get_stack_top(), get_heap_end(), get_stack_top() - get_heap_end());
	printf("starting mainloop...\r\n");

// 	doSnoop = true;
// 	diag->attach(this, &OpenOBC::printDS2Packet);
	
	Timer averageLitresPer100kmTimer(interruptManager);
	
	while(1)
	{
		if(!*stalkButton)
			lcd->printfClock("  :D");
		else
		{
			if(clockDisplayMode == CLOCKDISPLAY_CLOCK)
			{
				lcd->printfClock("%02i%02i", rtc->getHour(), rtc->getMinute());
			}
			else if(clockDisplayMode == CLOCKDISPLAY_DATE)
			{
				lcd->printfClock("%02i%02i", rtc->getMonth(), rtc->getDay());
			}
		}
		
		if(averageLitresPer100kmTimer.read_ms() >= 1000 && speed->getSpeed() > 0)
		{
			averageLitresPer100kmTimer.start();
			float litresPerHour = 0.2449 * 6 * 60 * fuelCons->getDutyCycle();
			float kilometresPerHour = speed->getSpeed();
			float litresPer100km = litresPerHour / kilometresPerHour * 100;
			if(averageFuelConsumptionSeconds > 0)
				averageLitresPer100km = (averageLitresPer100km * (averageFuelConsumptionSeconds - 1) + litresPer100km) / averageFuelConsumptionSeconds;
			else
				averageLitresPer100km = litresPer100km;
			averageFuelConsumptionSeconds++;
		}

		ccm->task();
		diag->task();
		callback->task();

		switch(displayMode)
		{
			case DISPLAY_VOLTAGE:
			{
				lcd->printf("%i %i  %.2fV", seti, setvalue, batteryVoltage->read());
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
				lcd->printf("%.1fC  %.0fF", temperature, temperature * 1.78 + 32);
				break;
			}
			case DISPLAY_TEMP1:
			{
				float temperature = -273.15;
				int ctraw = -1;
				
				DS2Packet* query = new DS2Packet(DS2_16BIT);
				query->setAddress(0x0d);
				uint8_t qdata[] = {0x08};
				query->setData(qdata, sizeof(qdata));
				
				DS2Packet* reply = diag->query(*query, DS2_L, 100);
				delete query;
				if(reply != NULL)
				{
					ctraw = reply->getData()[5];
					temperature = coolant_temp_table[ctraw];
					delete reply;
				}
				
				if(useMetricSystem)
					lcd->printf("coolant temp %.0fC", temperature);
				else
					lcd->printf("coolant temp %.0fF", temperature * 1.78 + 32);
				break;
			}
			case DISPLAY_CONSUM1:
			{
				if(useMetricSystem)
					lcd->printf("avg %2.1f L/100km", averageLitresPer100km);
				else
					lcd->printf("avg %2.1f mpg", 235.214f / averageLitresPer100km);
				break;
			}
			case DISPLAY_CONSUM2:
			{
				float litresPerHour = 0.2449 * 6 * 60 * fuelCons->getDutyCycle();
				float gallonsPerHour = litresPerHour / 3.78514;
				float kilometresPerHour = speed->getSpeed();
				float milesPerHour = kilometresPerHour * 0.621371;
				if(useMetricSystem)
					lcd->printf("%2.1f L/100km", litresPerHour / kilometresPerHour * 100);
				else
					lcd->printf("%2.1f mpg", milesPerHour / gallonsPerHour);
				break;
			}
			case DISPLAY_CONSUM3:
			{
				float litresPerMinute = 0.2449 * 6 * fuelCons->getDutyCycle();
				float gallonsPerMinute = litresPerMinute / 3.78514;
				if(useMetricSystem)
					lcd->printf("%1.3f L/min", litresPerMinute);
				else
					lcd->printf("%2.2f gal/hour", gallonsPerMinute * 60);
				break;
			}
			case DISPLAY_CONSUM4:
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
			case DISPLAY_RANGE1:
			{
				float range = fuelLevel->getLitres() / averageLitresPer100km * 100;
				if(useMetricSystem)
					lcd->printf("%.0f km  %.1f L", range, fuelLevel->getLitres());
				else
					lcd->printf("%.0f miles  %.2f gal", range * 0.621371f, fuelLevel->getGallons());
				break;
			}
			case DISPLAY_RANGE2:
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
			case DISPLAY_CLOCKSET:
			{
				lcd->printf("set clock");
				break;
			}
			case DISPLAY_DATESET:
			{
				lcd->printf("set date  %4i", rtc->getYear());
				break;
			}
			default:
			{
				lcd->printf("invalid displayMode");
				break;
			}	
		}
		
		out0->writeBits(out0Bits);
		out1->writeBits(out1Bits);

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

		delay(100);

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

void OpenOBC::button1000(bool isLast)
{
	if(displayMode == DISPLAY_CLOCKSET)
	{
		rtc->setHour(rtc->getHour() + 10);
	}
	else if(displayMode == DISPLAY_DATESET)
	{
		rtc->setMonth(rtc->getMonth() + 10);
	}
}

void OpenOBC::button100(bool isLast)
{
	if(displayMode == DISPLAY_CLOCKSET)
	{
		rtc->setHour(rtc->getHour() + 1);
	}
	else if(displayMode == DISPLAY_DATESET)
	{
		rtc->setMonth(rtc->getMonth() + 1);
	}
}

void OpenOBC::button10(bool isLast)
{
	if(displayMode == DISPLAY_CLOCKSET)
	{
		rtc->setMinute(rtc->getMinute() + 10);
	}
	else if(displayMode == DISPLAY_DATESET)
	{
		rtc->setDay(rtc->getDay() + 10);
	}
}

void OpenOBC::button1(bool isLast)
{
	if(displayMode == DISPLAY_CLOCKSET)
	{
		rtc->setMinute(rtc->getMinute() + 1);
	}
	else if(displayMode == DISPLAY_DATESET)
	{
		rtc->setDay(rtc->getDay() + 1);
	}
	else
	{
		displayMode = DISPLAY_OPENOBC;
	}
}

void OpenOBC::buttonConsum(bool isLast)
{
	if(displayMode == DISPLAY_CONSUM1)
		displayMode = DISPLAY_CONSUM2;
	else if(displayMode == DISPLAY_CONSUM2)
		displayMode = DISPLAY_CONSUM3;
	else if(displayMode == DISPLAY_CONSUM3)
		displayMode = DISPLAY_CONSUM4;
	else
		displayMode = DISPLAY_CONSUM1;
}

void OpenOBC::buttonRange(bool isLast)
{
	if(displayMode == DISPLAY_RANGE1)
		displayMode = DISPLAY_RANGE2;
	else
		displayMode = DISPLAY_RANGE1;
}

void OpenOBC::buttonTemp(bool isLast)
{
	if(displayMode == DISPLAY_TEMP)
		displayMode = DISPLAY_TEMP1;
	else
		displayMode = DISPLAY_TEMP;
}

void OpenOBC::buttonSpeed(bool isLast)
{
	displayMode = DISPLAY_SPEED;
}

void OpenOBC::buttonDist(bool isLast)
{
	displayMode = DISPLAY_VOLTAGE;
}

void OpenOBC::buttonTimer(bool isLast)
{
	doQuery = true;
}

void OpenOBC::buttonCheck(bool isLast)
{
	displayMode = DISPLAY_CHECK;
}

void OpenOBC::buttonKMMLS(bool isLast)
{
	useMetricSystem = !useMetricSystem;
}

void OpenOBC::buttonClock(bool isLast)
{
	clockDisplayMode = CLOCKDISPLAY_CLOCK;
}

void OpenOBC::buttonDate(bool isLast)
{
	clockDisplayMode = CLOCKDISPLAY_DATE;
}

void OpenOBC::buttonMemo(bool isLast)
{
	displayMode = DISPLAY_FREEMEM;
}

void OpenOBC::buttonSet(bool isLast)
{
	if(keypad->getKeys() & BUTTON_CLOCK_MASK)
	{
		if(displayMode != DISPLAY_CLOCKSET)
			lastDisplayMode = displayMode;
		displayMode = DISPLAY_CLOCKSET;
	}
	else if(keypad->getKeys() & BUTTON_DATE_MASK)
	{
		if((displayMode != DISPLAY_CLOCKSET) && (displayMode != DISPLAY_DATESET))
			lastDisplayMode = displayMode;
		displayMode = DISPLAY_DATESET;
	}
	else if(displayMode == DISPLAY_CLOCKSET)
	{
		displayMode = lastDisplayMode;
	}
	else if(displayMode == DISPLAY_DATESET)
	{
		displayMode = lastDisplayMode;
	}
	else if(displayMode == DISPLAY_OPENOBC)
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
	else if(displayMode == DISPLAY_CONSUM1)
	{
		averageFuelConsumptionSeconds = 0;
		averageLitresPer100km = 0;
	}
}

void runHandler(bool isLast)
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

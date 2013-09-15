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
#include <PCA95xxPin.h>
#include <ObcCode.h>
#include <ObcCheck.h>
#include <ObcLimit.h>
#include <ObcSpeed.h>
#include <ObcConsum.h>
#include <ObcDist.h>
#include <ObcRange.h>
#include <ObcTemp.h>
#include <ObcTimer.h>
#include <ObcKmmls.h>
#include <ObcMemo.h>
#include <ObcOdometer.h>
#include "ObcUI.h"

volatile uint32_t SysTickCnt;
OpenOBC* obcS;
Debug* debugS;
RTC* rtcS;

bool doSleep = false;
bool doSleepCheck = false;
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
			
// 			//check packet and send reply
// 			if((packet->getType() == DS2_16BIT) && (packet->getDataLength() == 1))
// 			{
// 				char data = packet->getData()[0];
// 				const uint8_t reply0[] = {0x0d, 0x00, 0x0b, 0x00, 0x03, 0x37, 0xbe, 0x25, 0x02, 0x94, 0x3f};
// // 				reply8[5] = setvalue; //set 58g high time (raw value)
// // 				reply8[6] = setvalue; //set 58g low time (raw value)
// // 				reply8[7] = setvalue; //set clamp 30 voltage
// // 				reply8[8] = setvalue; //set coolant temperature
// // 				reply8[9] = setvalue; //set fuel capacity
// // 				reply8[10] = setvalue; //set ???
// // 				reply8[11] = setvalue; //set ec signal
// // 				reply8[12] = setvalue; //set ec signal
// // 				reply8[13] = setvalue; //set rpm
// // 				reply8[14] = setvalue; //set rpm
// // 				reply8[15] = setvalue; //set speed
// // 				reply8[16] = setvalue; //set speed
// // 				reply8[17] = setvalue; //set digital values
// 
// 				reply8[seti] = setvalue;
// 				
// 				if(data == 0x00)
// 				{
// 					DS2Packet* reply = new DS2Packet(reply0, 11, DS2_16BIT);
// 					diag->write(*reply, DS2_K);
// 					delete reply;
// 					DEBUG("replied to 0\r\n");
// 				}
// 				else if(data == 0x08)
// 				{
// 					DS2Packet* reply = new DS2Packet(reply8, 0x17, DS2_16BIT);
// 					diag->write(*reply, DS2_K);
// 					delete reply;
// 					DEBUG("replied to 8\r\n");
// 				}
// 			}
// 			else if(((packet->getType() == DS2_16BIT) && (packet->getDataLength() == 5)))
// 			{
// 				uint8_t* data = packet->getData();
// 				const uint8_t replyc[] = {0x0d, 0x00, 0x07, 0x0c, 0x97, 0x02, 0x93};
// 				const uint8_t replyc1[] = {0x0d, 0x00, 0x07, 0x0c, 0x73, 0x58, 0x2d};
// 				if(data[0] == 0x0c && data[4] == 0x00)
// 				{
// 					DS2Packet* reply = new DS2Packet(replyc, 7, DS2_16BIT);
// 					diag->write(*reply, DS2_K);
// 					delete reply;
// 					DEBUG("replied to c\r\n");
// 				}
// 				else if(data[0] == 0x0c && data[4] == 0x10)
// 				{
// 					DS2Packet* reply = new DS2Packet(replyc1, 7, DS2_16BIT);
// 					diag->write(*reply, DS2_K);
// 					delete reply;
// 					DEBUG("replied to c1\r\n");
// 				}
// 			}
			
			delete packet;
		}
		else
			break;
	}
}

void OpenOBC::writeConfigData()
{
	if(this->batteryVoltageCalibration != (float)atof(config->getValueByName("BatteryVoltageCalibration").c_str()))
	{
		char* newValue = new char[8];
		snprintf(newValue, 8, "%.4f", batteryVoltageCalibration);
		config->setValueByName("BatteryVoltageCalibration", newValue);
		delete[] newValue;
	}
}

OpenOBC::OpenOBC() : displayMode(reinterpret_cast<volatile DisplayMode_Type&>(LPC_RTC->GPREG0)), averageLitresPer100km(reinterpret_cast<volatile float&>(LPC_RTC->GPREG1)), averageFuelConsumptionSeconds(reinterpret_cast<volatile uint32_t&>(LPC_RTC->GPREG2))
{
	wdt.start(10);
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
	
	__enable_irq();
	SysTick_Config(SystemCoreClock/1000 - 1); //interrupt period 1ms
	debug = new Debug(DEBUG_TX_PORTNUM, DEBUG_TX_PINNUM, DEBUG_RX_PORTNUM, DEBUG_RX_PINNUM, DEBUG_BAUD, &interruptManager); debugS = debug;

// 	printf("-"); fflush(stdout); delay(250); printf("-"); fflush(stdout); delay(250); printf("-"); fflush(stdout); delay(250); printf(">\r\n"); delay(250);
	printf("--->\r\n");
	printf("clock speed: %i MHz\r\n", SystemCoreClock / 1000000);
	printf("stack: 0x%lx heap: 0x%lx free: %li\r\n", get_stack_top(), get_heap_end(), get_mem_free());

	vrefEn = new IO(VREF_EN_PORT, VREF_EN_PIN, true, false);
	
	//spi coniguration
	spi0 = new SPI(SPI0_MOSI_PORT, SPI0_MOSI_PIN, SPI0_MISO_PORT, SPI0_MISO_PIN, SPI0_SCK_PORT, SPI0_SCK_PIN);
	spi1 = new SPI(SPI1_MOSI_PORT, SPI1_MOSI_PIN, SPI1_MISO_PORT, SPI1_MISO_PIN, SPI1_SCK_PORT, SPI1_SCK_PIN);
	
	//i2c configuration
	i2c0 = new I2C(I2C0_SDA_PORT, I2C0_SDA_PIN, I2C0_SCL_PORT, I2C0_SCL_PIN);
	i2c1 = new I2C(I2C1_SDA_PORT, I2C1_SDA_PIN, I2C1_SCL_PORT, I2C1_SCL_PIN);
	
	//i/o expander configuration
	Input* io0Interrupt = new Input(PCA95XX_INTERRUPT_PORT, PCA95XX_INTERRUPT_PIN);
	io0 = new PCA95xx(*i2c0, PCA95XX_ADDRESS, *io0Interrupt, 400000);
	codeLed = new PCA95xxPin(*io0, CODE_LED_PORT, CODE_LED_PIN, true);
	limitLed = new PCA95xxPin(*io0, LIMIT_LED_PORT, LIMIT_LED_PIN, true);
	timerLed = new PCA95xxPin(*io0, TIMER_LED_PORT, TIMER_LED_PIN, true);
	ccmLight = new PCA95xxPin(*io0, CCM_LIGHT_PORT, CCM_LIGHT_PIN, true);
	chime0 = new PCA95xxPin(*io0, CHIME0_PORT, CHIME0_PIN, true);
	chime1 = new PCA95xxPin(*io0, CHIME1_PORT, CHIME1_PIN, true);
	ventilation = new PCA95xxPin(*io0, VENTILATION_PORT, VENTILATION_PIN, true);
	antitheftHorn = new PCA95xxPin(*io0, ANTITHEFT_HORN_PORT, ANTITHEFT_HORN_PIN, true);
	ews = new PCA95xxPin(*io0, EWS_PORT, EWS_PIN, true);
	
	//lcd configuration
	lcdBiasEn = new PCA95xxPin(*io0, LCD_BIAS_EN_PORT, LCD_BIAS_EN_PIN, true);
	lcdReset = new IO(LCD_RESET_PORT, LCD_RESET_PIN, false);
// 	IO lcdBias(LCD_BIASCLOCK_PORT, LCD_BIASCLOCK_PIN, true);
	lcdBiasClock = new PWM(LCD_BIASCLOCK_PORT, LCD_BIASCLOCK_PIN, .99);
	IO* lcdSel = new IO(LCD_SELECT_PORT, LCD_SELECT_PIN, true);
	IO* lcdRefresh = new IO(LCD_REFRESH_PORT, LCD_REFRESH_PIN, false);
	IO* lcdUnk0 = new IO(LCD_UNK0_PORT, LCD_UNK0_PIN, true);
	IO* lcdUnk1 = new IO(LCD_UNK1_PORT, LCD_UNK1_PIN, false);
	lcd = new ObcLcd(*spi1, *lcdSel, *lcdRefresh, *lcdUnk0, *lcdUnk1);
	*lcdReset = true;
	*lcdBiasEn = true;
	
	//backlight configuration
	lcdLight = new IO(LCD_BACKLIGHT_PORT, LCD_BACKLIGHT_PIN, true);
	clockLight = new IO(CLOCK_BACKLIGHT_PORT, CLOCK_BACKLIGHT_PIN, true);
	keypadLight = new IO(KEYPAD_BACKLIGHT_PORT, KEYPAD_BACKLIGHT_PIN, true);
// 	lcdBacklight = new PWM(LCD_BACKLIGHT_PORT, LCD_BACKLIGHT_PIN, .2);
// 	clockBacklight = new PWM(CLOCK_BACKLIGHT_PORT, CLOCK_BACKLIGHT_PIN);
	
	lcd->printf("read config file...");
	
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
			DEBUG("created new config file: %s\r\n", config->getFilename().c_str());
		}
		else
		{
			DEBUG("couldn't open file for writing: %s\r\n", config->getFilename().c_str());
		}
	}
	
	if(config->getValueByName("LogConsoleToFile") == "yes")
		freopen("/sd/stdout.log", "a", stdout);
	else
		config->setValueByName("LogConsoleToFile", "no");
	
	//default config file parameters
	if(config->getValueByName("DefaultDisplayMode") == "")
		config->setValueByName("DefaultDisplayMode", "DISPLAY_LAST_DISPLAYMODE");
	if(config->getValueByName("BatteryVoltageCalibration") == "")
		config->setValueByName("BatteryVoltageCalibration", "1.0000");	

	//default display mode configuration
	std::string defaultDisplayModeString = config->getValueByName("DefaultDisplayMode");
	if(defaultDisplayModeString == "DISPLAY_LAST_DISPLAYMODE")
		displayMode = displayMode;
	else if(defaultDisplayModeString == "DISPLAY_CHECK")
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
	else if(defaultDisplayModeString == "DISPLAY_ACCELEROMETER")
		displayMode = DISPLAY_ACCELEROMETER;
	else if(defaultDisplayModeString == "DISPLAY_OPENOBC")
		displayMode = DISPLAY_OPENOBC;
	else if(defaultDisplayModeString == "DISPLAY_SPEED")
		displayMode = DISPLAY_SPEED;
	else if(defaultDisplayModeString == "DISPLAY_TEMP")
		displayMode = DISPLAY_TEMP;
	else if(defaultDisplayModeString == "DISPLAY_VOLTAGE")
		displayMode = DISPLAY_VOLTAGE;
	else
		displayMode = DISPLAY_OPENOBC;

	clockDisplayMode = CLOCKDISPLAY_CLOCK;
	
	batteryVoltageCalibration = atof(config->getValueByName("BatteryVoltageCalibration").c_str());
	
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
	
	//accelerometer configuration
	Input* accelInterrupt = new Input(ACCEL_INTERRUPT_PORT, ACCEL_INTERRUPT_PIN);
	accel = new MMA845x(*i2c0, ACCEL_ADDRESS, *accelInterrupt, interruptManager);
	
	//ccm configuration
	Input* ccmData = new Input(CCM_DATA_PORT, CCM_DATA_PIN);
	IO* ccmClock = new IO(CCM_CLOCK_PORT, CCM_CLOCK_PIN);
	IO* ccmLatch = new IO(CCM_LATCH_PORT, CCM_LATCH_PIN);
	uint8_t ccmDisableMask = strtoul(config->getValueByName("ObcCheckDisableMask").c_str(), NULL, 0);
	if(ccmDisableMask == 0)
	{
		config->setValueByName("ObcCheckDisableMask", "0x%02x", DEFAULT_CCM_DISABLE_MASK);
		ccmDisableMask = DEFAULT_CCM_DISABLE_MASK;
	}
	uint8_t ccmInvertMask = strtoul(config->getValueByName("ObcCheckInvertMask").c_str(), NULL, 0);
	if(ccmInvertMask == 0)
	{
		config->setValueByName("ObcCheckInvertMask", "0x%02x", DEFAULT_CCM_INVERT_MASK);
		ccmInvertMask = DEFAULT_CCM_INVERT_MASK;
	}
	
	ccm = new CheckControlModule(*ccmData, *ccmClock, *ccmLatch, ccmDisableMask, ccmInvertMask);

	//fuel level configuration
	Input* fuelLevelInput = new Input(FUEL_LEVEL_PORT, FUEL_LEVEL_PIN);
	fuelLevel = new FuelLevel(*fuelLevelInput, interruptManager);

	//stalk button configuration
	stalkButton = new Input(STALK_BUTTON_PORT, STALK_BUTTON_PIN);

	//diagnostics interface configuration
	klWake = new IO(KL_WAKE_PORT, KL_WAKE_PIN, true);
	kline = new Uart(KLINE_TX_PORTNUM, KLINE_TX_PINNUM, KLINE_RX_PORTNUM, KLINE_RX_PINNUM, KLINE_BAUD, UART_PARITY_EVEN, &interruptManager);
	lline = new Uart(LLINE_TX_PORTNUM, LLINE_TX_PINNUM, LLINE_RX_PORTNUM, LLINE_RX_PINNUM, LLINE_BAUD, UART_PARITY_EVEN, &interruptManager);
	DS2Bus* k = new Bus(*kline);
	DS2Bus* l = new Bus(*lline);
	diag = new DS2(*k, *l);
	disableComms = false;

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
// 	speedPin->setPullup(); //for bench use only
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
	keypad->attach(BUTTON_CLOCK, this, &OpenOBC::buttonClock);
	keypad->attach(BUTTON_DATE, this, &OpenOBC::buttonDate);
	keypad->attach(BUTTON_SET, this, &OpenOBC::buttonSet);

	//analog input configuration
	batteryVoltage = new AnalogIn(BATTERY_VOLTAGE_PORT, BATTERY_VOLTAGE_PIN, REFERENCE_VOLTAGE, (10 + 1.0) / 1.0 * REFERENCE_VOLTAGE, atof(config->getValueByName("BatteryVoltageCalibration").c_str()));
	temperature = new AnalogIn(EXT_TEMP_PORT,EXT_TEMP_PIN, REFERENCE_VOLTAGE);
	analogIn1 = new AnalogIn(ANALOG_IN1_PORT, ANALOG_IN1_PIN);
	analogIn2 = new AnalogIn(ANALOG_IN2_PORT, ANALOG_IN2_PIN);
	
	//analog output configuration
	analogOut = new AnalogOut(ANALOG_OUT_PORT, ANALOG_OUT_PIN, REFERENCE_VOLTAGE);
	analogOut->writeVoltage(1.0);
	
// 	averageFuelConsumptionSeconds = 0;
// 	averageLitresPer100km = 0;

	ccmLight->on();
	codeLed->on();
	limitLed->on();
	timerLed->on();
	if(wdt.wasReset())
	{
		printf("WATCHDOG RESET\r\n");
		lcd->printfClock("!!!!");
		Timer flashTimer;
		while((keypad->getKeys() & BUTTON_SET_MASK) != BUTTON_SET_MASK)
		{
			if(flashTimer.read_ms() <= 1500)
			{
				lcd->printf("WATCHDOG RESET");
			}
			else if(flashTimer.read_ms() <= 3000)
			{
				lcd->printf("push SET to continue");
			}
			else
			{
				flashTimer.start();
			}
			wdt.feed();
		}
		lcd->clear();
		lcd->clearClock();
	}
	printf("openOBC firmware version: %s\r\n", GIT_VERSION);
	lcd->printf("openOBC %s", GIT_VERSION);
	lcd->printfClock("%s", GIT_TAG);
	delay(1500);
	lcd->printf("");
	lcd->printfClock("");
	ccmLight->off();
	codeLed->off();
	limitLed->off();
	timerLed->off();
	
	coolantTemperature = 0;
	
	ui = new ObcUI(*lcd, *keypad, *config);
	keypad->attachRaw(ui, &ObcUI::handleButtonEvent);
	ui->addTask(new ObcOdometer(*this));
	ui->addTask(new ObcSpeed(*this));
	ui->addTask(new ObcMemo(*this));
	ui->addTask(new ObcTemp(*this));
	ui->addTask(new ObcCheck(*this));
	ui->addTask(new ObcLimit(*this));
	ui->addTask(new ObcConsum(*this));
	ui->addTask(new ObcDist(*this));
	ui->addTask(new ObcRange(*this));
	ui->addTask(new ObcTimer(*this));
	ui->addTask(new ObcKmmls(*this));
	ui->addTask(new ObcCode(*this));
	
	ui->wake();
	
	doSleepCheck = true;
}

void OpenOBC::uartHandler()
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
			{
				batteryVoltageCalibration += 0.0005;
				batteryVoltage->setCalibrationScale(batteryVoltageCalibration);
			}
			else if(c == 'd')
			{
				batteryVoltageCalibration -= 0.0005;
				batteryVoltage->setCalibrationScale(batteryVoltageCalibration);
			}
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
	debug->attach(this, &OpenOBC::uartHandler);
	
	printf("stack: 0x%lx heap: 0x%lx free: %li\r\n", get_stack_top(), get_heap_end(), get_mem_free());
	printf("starting mainloop...\r\n");
	wdt.start(5);

	Timer coolantTemperatureTimer(interruptManager);
	Timer displayRefreshTimer(interruptManager);
	
	while(1)
	{
		wdt.feed();
		
		if(!disableComms)
		{
			if(coolantTemperatureTimer.read_ms() >= 1000)
			{
				coolantTemperatureTimer.start();
				coolantTemperature = kombi->getCoolantTemperature();
			}
		}
		else
		{
			coolantTemperature = 0;
		}
		
		ui->task();
		ccm->task();
		diag->task();
		callback->task();
		
		if(displayRefreshTimer.read_ms() >= 100)
		{
			displayRefreshTimer.start();
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
			
			if(0)
			switch(displayMode)
			{
				case DISPLAY_CALIBRATE:
				{
					lcd->printf("set %.2fV", batteryVoltage->read());
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
		}
		
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

		static Timer sleepTimer;
		if(!doSleep)
		{
			sleepTimer.start();
		}
		else
		{
			if(sleepTimer.read_ms() >= 1000)
			{
// 				while(doSleep)
					sleep();
				wake();
			}
		}
	}

}

void OpenOBC::sleep()
{
	DEBUG("writing config file...\r\n");
	lcd->printf("write config file...");
	writeConfigData();
	ui->sleep();
	if(!doSleep)
		return;
	printf("sleeping...\r\n");
	lcd->printf("sleeping...");
	*lcdBiasEn = false;
	*lcdReset = false;
	*lcdLight = false;
	*clockLight = false;
	*keypadLight = false;
	*klWake = false;
	*vrefEn = false;
	accel->disable();

	fclose(stdout);

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
	__disable_irq();
// 	*obcS->clockLight = true;
// 	delay(160); //80 = 2 seconds at 4MHz
// 	SystemInit();
// 	*obcS->lcdLight = true;
// 	*obcS->keypadLight = true;
	__set_MSP(0x10008000);
	Reset_Handler();
}

void OpenOBC::button1000()
{
	if(displayMode == DISPLAY_CLOCKSET)
	{
		int hour = rtc->getHour() + 10;
		if(hour >= 24)
			while(hour >= 10)
				hour -= 10;
		rtc->setHour(hour);
	}
	else if(displayMode == DISPLAY_DATESET)
	{
		int month = rtc->getMonth() + 10;
		if(month >= 13)
			while(month >= 10)
				month -= 10;
		if(month == 0)
			month = 10;
		rtc->setMonth(month);
	}
	else if(displayMode == DISPLAY_OPENOBC)
	{
		disableComms = !disableComms;
	}
}

void OpenOBC::button100()
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

void OpenOBC::button10()
{
	if(displayMode == DISPLAY_CLOCKSET)
	{
		int minute = rtc->getMinute() + 10;
		if(minute >= 60)
			while(minute >= 10)
				minute -= 10;
		rtc->setMinute(minute);
	}
	else if(displayMode == DISPLAY_DATESET)
	{
		int day = rtc->getDay() + 10;
		if(day >= 32)
			while(day >= 10)
				day -= 10;
		if(day == 0)
			day = 10;
		rtc->setDay(day);
	}
	else if(displayMode == DISPLAY_CALIBRATE)
	{
		batteryVoltageCalibration += 0.0005;
		batteryVoltage->setCalibrationScale(batteryVoltageCalibration);
	}
}

void OpenOBC::button1()
{
	if(displayMode == DISPLAY_CLOCKSET)
	{
		rtc->setMinute(rtc->getMinute() + 1);
	}
	else if(displayMode == DISPLAY_DATESET)
	{
		rtc->setDay(rtc->getDay() + 1);
	}
	else if(displayMode == DISPLAY_CALIBRATE)
	{
		batteryVoltageCalibration -= 0.0005;
		batteryVoltage->setCalibrationScale(batteryVoltageCalibration);
	}
}

void OpenOBC::buttonClock()
{
	if(displayMode == DISPLAY_DATESET)
		displayMode = DISPLAY_CLOCKSET;
	clockDisplayMode = CLOCKDISPLAY_CLOCK;
}

void OpenOBC::buttonDate()
{
	if(displayMode == DISPLAY_CLOCKSET)
		displayMode = DISPLAY_DATESET;
	clockDisplayMode = CLOCKDISPLAY_DATE;
}

void OpenOBC::buttonSet()
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
	else if(displayMode == DISPLAY_VOLTAGE)
	{
		batteryVoltageCalibration = atof(config->getValueByName("BatteryVoltageCalibration").c_str());
		displayMode = DISPLAY_CALIBRATE;
	}
	else if(displayMode == DISPLAY_CALIBRATE)
	{
		displayMode = DISPLAY_VOLTAGE;
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

	if(doSleepCheck)
	{
		if(*obcS->run)
			doSleep = false;
		else
			doSleep = true;
	}
}

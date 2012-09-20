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

#include "InterruptManager.h"
#include <lpc17xx_gpio.h>
#include <lpc17xx_pinsel.h>

#define WEAK __attribute__ ((weak))
// #define WEAK

InterruptManagerOwner* interruptManagerOwner;

InterruptManager::InterruptManager(InterruptManagerOwner* owner)
{
	interruptManagerOwner = owner;
	for(uint32_t i = 0; i < IRQHandler_NUM_VALUES; i++)
		pointers[i] = 0;

	GPIO_ClearValue(2, (1<<0));
	GPIO_SetDir(2, (1<<0), 1);
}


extern "C" void WEAK NonMaskableInt_Handler() {interruptManagerOwner->interruptManager.call(IRQ_NonMaskableInt);}
extern "C" void WEAK MemoryManagement_Handler() {interruptManagerOwner->interruptManager.call(IRQ_MemoryManagement);}
extern "C" void WEAK BusFault_Handler() {interruptManagerOwner->interruptManager.call(IRQ_BusFault);}
extern "C" void WEAK UsageFault_Handler() {interruptManagerOwner->interruptManager.call(IRQ_UsageFault);}
extern "C" void WEAK SVCall_Handler() {interruptManagerOwner->interruptManager.call(IRQ_SVCall);}
extern "C" void WEAK DebugMonitor_Handler() {interruptManagerOwner->interruptManager.call(IRQ_DebugMonitor);}
extern "C" void WEAK PendSV_Handler() {interruptManagerOwner->interruptManager.call(IRQ_PendSV);}
extern "C" void WEAK SysTick_Handler() {interruptManagerOwner->interruptManager.call(IRQ_SysTick);}
extern "C" void WEAK WDT_IRQHandler() {interruptManagerOwner->interruptManager.call(IRQ_WDT);}
extern "C" void WEAK TIMER0_IRQHandler() {interruptManagerOwner->interruptManager.call(IRQ_TIMER0);}
extern "C" void WEAK TIMER1_IRQHandler() {interruptManagerOwner->interruptManager.call(IRQ_TIMER1);}
extern "C" void WEAK TIMER2_IRQHandler() {interruptManagerOwner->interruptManager.call(IRQ_TIMER2);}
extern "C" void WEAK TIMER3_IRQHandler() {interruptManagerOwner->interruptManager.call(IRQ_TIMER3);}
extern "C" void WEAK UART0_IRQHandler() {interruptManagerOwner->interruptManager.call(IRQ_UART0);}
extern "C" void WEAK UART1_IRQHandler() {interruptManagerOwner->interruptManager.call(IRQ_UART1);}
extern "C" void WEAK UART2_IRQHandler() {interruptManagerOwner->interruptManager.call(IRQ_UART2);}
extern "C" void WEAK UART3_IRQHandler() {interruptManagerOwner->interruptManager.call(IRQ_UART3);}
extern "C" void WEAK PWM1_IRQHandler() {interruptManagerOwner->interruptManager.call(IRQ_PWM1);}
extern "C" void WEAK I2C0_IRQHandler() {interruptManagerOwner->interruptManager.call(IRQ_I2C0);}
extern "C" void WEAK I2C1_IRQHandler() {interruptManagerOwner->interruptManager.call(IRQ_I2C1);}
extern "C" void WEAK I2C2_IRQHandler() {interruptManagerOwner->interruptManager.call(IRQ_I2C2);}
extern "C" void WEAK SPI_IRQHandler() {interruptManagerOwner->interruptManager.call(IRQ_SPI);}
extern "C" void WEAK SSP0_IRQHandler() {interruptManagerOwner->interruptManager.call(IRQ_SSP0);}
extern "C" void WEAK SSP1_IRQHandler() {interruptManagerOwner->interruptManager.call(IRQ_SSP1);}
extern "C" void WEAK PLL0_IRQHandler() {interruptManagerOwner->interruptManager.call(IRQ_PLL0);}
extern "C" void WEAK RTC_IRQHandler() {interruptManagerOwner->interruptManager.call(IRQ_RTC);}
extern "C" void WEAK EINT0_IRQHandler() {interruptManagerOwner->interruptManager.call(IRQ_EINT0);}
extern "C" void WEAK EINT1_IRQHandler() {interruptManagerOwner->interruptManager.call(IRQ_EINT1);}
extern "C" void WEAK EINT2_IRQHandler() {interruptManagerOwner->interruptManager.call(IRQ_EINT2);}
extern "C" void WEAK EINT3_IRQHandler() {interruptManagerOwner->interruptManager.call(IRQ_EINT3);}
extern "C" void WEAK ADC_IRQHandler() {interruptManagerOwner->interruptManager.call(IRQ_ADC);}
extern "C" void WEAK BOD_IRQHandler() {interruptManagerOwner->interruptManager.call(IRQ_BOD);}
extern "C" void WEAK USB_IRQHandler() {interruptManagerOwner->interruptManager.call(IRQ_USB);}
extern "C" void WEAK CAN_IRQHandler() {interruptManagerOwner->interruptManager.call(IRQ_CAN);}
extern "C" void WEAK DMA_IRQHandler() {interruptManagerOwner->interruptManager.call(IRQ_DMA);}
extern "C" void WEAK I2S_IRQHandler() {interruptManagerOwner->interruptManager.call(IRQ_I2S);}
extern "C" void WEAK ENET_IRQHandler() {interruptManagerOwner->interruptManager.call(IRQ_ENET);}
extern "C" void WEAK RIT_IRQHandler() {interruptManagerOwner->interruptManager.call(IRQ_RIT);}
extern "C" void WEAK MCPWM_IRQHandler() {interruptManagerOwner->interruptManager.call(IRQ_MCPWM);}
extern "C" void WEAK QEI_IRQHandler() {interruptManagerOwner->interruptManager.call(IRQ_QEI);}
extern "C" void WEAK PLL1_IRQHandler() {interruptManagerOwner->interruptManager.call(IRQ_PLL1);}
extern "C" void WEAK USBActivity_IRQHandler() {interruptManagerOwner->interruptManager.call(IRQ_USBActivity);}
extern "C" void WEAK CANActivity_IRQHandler() {interruptManagerOwner->interruptManager.call(IRQ_CANActivity);}

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

#ifndef INTERRUPTMANAGER_H
#define INTERRUPTMANAGER_H

#include "FunctionPointer.h"
#include <cstdio>
#include <IO.h> //FIXME remove
extern IO* isr; //FIXME remove
extern IO* idle; //FIXME remove

typedef enum
{
	/******  Cortex-M3 Processor Exceptions Numbers ***************************************************/
	IRQ_NonMaskableInt           = -14,      /*!< 2 Non Maskable Interrupt                         */
	IRQ_MemoryManagement         = -12,      /*!< 4 Cortex-M3 Memory Management Interrupt          */
	IRQ_BusFault                 = -11,      /*!< 5 Cortex-M3 Bus Fault Interrupt                  */
	IRQ_UsageFault               = -10,      /*!< 6 Cortex-M3 Usage Fault Interrupt                */
	IRQ_SVCall                   = -5,       /*!< 11 Cortex-M3 SV Call Interrupt                   */
	IRQ_DebugMonitor             = -4,       /*!< 12 Cortex-M3 Debug Monitor Interrupt             */
	IRQ_PendSV                   = -2,       /*!< 14 Cortex-M3 Pend SV Interrupt                   */
	IRQ_SysTick                  = -1,       /*!< 15 Cortex-M3 System Tick Interrupt               */
	
	/******  LPC17xx Specific Interrupt Numbers *******************************************************/
	IRQ_WDT                      = 0,        /*!< Watchdog Timer Interrupt                         */
	IRQ_TIMER0                   = 1,        /*!< Timer0 Interrupt                                 */
	IRQ_TIMER1                   = 2,        /*!< Timer1 Interrupt                                 */
	IRQ_TIMER2                   = 3,        /*!< Timer2 Interrupt                                 */
	IRQ_TIMER3                   = 4,        /*!< Timer3 Interrupt                                 */
	IRQ_UART0                    = 5,        /*!< UART0 Interrupt                                  */
	IRQ_UART1                    = 6,        /*!< UART1 Interrupt                                  */
	IRQ_UART2                    = 7,        /*!< UART2 Interrupt                                  */
	IRQ_UART3                    = 8,        /*!< UART3 Interrupt                                  */
	IRQ_PWM1                     = 9,        /*!< PWM1 Interrupt                                   */
	IRQ_I2C0                     = 10,       /*!< I2C0 Interrupt                                   */
	IRQ_I2C1                     = 11,       /*!< I2C1 Interrupt                                   */
	IRQ_I2C2                     = 12,       /*!< I2C2 Interrupt                                   */
	IRQ_SPI                      = 13,       /*!< SPI Interrupt                                    */
	IRQ_SSP0                     = 14,       /*!< SSP0 Interrupt                                   */
	IRQ_SSP1                     = 15,       /*!< SSP1 Interrupt                                   */
	IRQ_PLL0                     = 16,       /*!< PLL0 Lock (Main PLL) Interrupt                   */
	IRQ_RTC                      = 17,       /*!< Real Time Clock Interrupt                        */
	IRQ_EINT0                    = 18,       /*!< External Interrupt 0 Interrupt                   */
	IRQ_EINT1                    = 19,       /*!< External Interrupt 1 Interrupt                   */
	IRQ_EINT2                    = 20,       /*!< External Interrupt 2 Interrupt                   */
	IRQ_EINT3                    = 21,       /*!< External Interrupt 3 Interrupt                   */
	IRQ_ADC                      = 22,       /*!< A/D Converter Interrupt                          */
	IRQ_BOD                      = 23,       /*!< Brown-Out Detect Interrupt                       */
	IRQ_USB                      = 24,       /*!< USB Interrupt                                    */
	IRQ_CAN                      = 25,       /*!< CAN Interrupt                                    */
	IRQ_DMA                      = 26,       /*!< General Purpose DMA Interrupt                    */
	IRQ_I2S                      = 27,       /*!< I2S Interrupt                                    */
	IRQ_ENET                     = 28,       /*!< Ethernet Interrupt                               */
	IRQ_RIT                      = 29,       /*!< Repetitive Interrupt Timer Interrupt             */
	IRQ_MCPWM                    = 30,       /*!< Motor Control PWM Interrupt                      */
	IRQ_QEI                      = 31,       /*!< Quadrature Encoder Interface Interrupt           */
	IRQ_PLL1                     = 32,       /*!< PLL1 Lock (USB PLL) Interrupt                    */
	IRQ_USBActivity              = 33,	     /*!< USB Activity Interrupt                           */
	IRQ_CANActivity              = 34,       /*!< CAN Activity Interrupt                           */
	
	IRQHandler_NUM_VALUES        = 43        /*!< Number of IRQHandler_Type values                 */
	
} IRQHandler_Type;

class InterruptManagerOwner;

class InterruptManager
{
public:
    InterruptManager(InterruptManagerOwner* owner);

	template<typename T>
	void attach(IRQHandler_Type irq, T* classPointer, void (T::*methodPointer)(void))
	{
		if((methodPointer != 0) && (classPointer != 0))
		{
			if(!pointers[irq])
				pointers[irq] = new FunctionPointer();
			pointers[irq]->attach(classPointer, methodPointer);
		}
	}
	template<typename T>
	void detach(IRQHandler_Type irq, T* classPointer, void (T::*methodPointer)(void))
	{
		if((methodPointer != 0) && (classPointer != 0))
		{
			if(pointers[irq])
				pointers[irq]->detach(classPointer, methodPointer);
		}
	}
	
	void attach(IRQHandler_Type irq, void (*functionPointer)(void))
	{
		if(!pointers[irq])
			pointers[irq] = new FunctionPointer;
		pointers[irq]->attach(functionPointer);
	}
	void detach(IRQHandler_Type irq, void (*functionPointer)(void))
	{
		if(pointers[irq])
			pointers[irq]->detach(functionPointer);
	}
	
	void call(IRQHandler_Type irq)
	{
		isr->on(); //FIXME remove
// 		idle->on(); //FIXME remove
		if(irq < 0)
			throw(0); //FIXME remove
		
		if(pointers[irq]) //FIXME enum values do not map completely to a congiguous array
		{
			pointers[irq]->call(); //FIXME enum values do not map completely to a congiguous array
		}

// 		idle->off(); //FIXME remove
		isr->off(); //FIXME remove
	}

	bool isAttached(IRQHandler_Type irq)
	{
		return pointers[irq]->isValid();
	}

private:
	FunctionPointer* pointers[IRQHandler_NUM_VALUES]; //FIXME enum values do not map completely to a congiguous array
};

class InterruptManagerOwner
{
public:
	InterruptManagerOwner() : interruptManager(this) {};
	InterruptManager interruptManager;
};

#endif // INTERRUPTMANAGER_H

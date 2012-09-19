#include "delay.h"

void delay(uint32_t ms)
{
	uint32_t systickcnt;
	
	systickcnt = SysTickCnt;
	while((SysTickCnt - systickcnt) < ms);
}

#include "delay.h"
#include "IO.h"

extern IO* idle;

void delay(uint32_t ms)
{
	idle->off();
	uint32_t systickcnt;
	systickcnt = SysTickCnt;
	while((SysTickCnt - systickcnt) < ms);
	idle->on();
}

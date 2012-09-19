#ifndef DELAY_H
#define DELAY_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif //__cplusplus
	
extern volatile uint32_t SysTickCnt;

void delay(uint32_t ms);


#ifdef __cplusplus
}
#endif //__cplusplus

#endif //DELAY_H

#include "mcu_support_package/inc/stm32f10x.h"
#include "main/bitmagic.h"



uint8_t countSetBits( uint32_t n )
{
n = ((n>>1) & 0x55555555) + (n & 0x55555555);
n = ((n>>2) & 0x33333333 ) + (n & 0x33333333);
n = ((((n>>4) & 0x0F0F0F0F) + (n & 0x0F0F0F0F)) * 0x01010101) >> 24;
return n; 
}

uint8_t countLeadingZeros( uint32_t n )
{
	return __CLZ(n); 
}


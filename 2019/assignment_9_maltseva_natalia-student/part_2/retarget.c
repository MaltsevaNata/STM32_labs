#pragma import(__use_no_semihosting_swi)


#include <stdio.h>
#include <rt_sys.h>
#include <rt_misc.h>
#include "mcu_support_package/inc/stm32f10x.h"

// we need to define this struct, which will be typedefed as FILE in <stdio.h>
struct __FILE { 
	int handle;
};

// we need to declare this variables to avoid their allocation on heap
FILE __stdout;
FILE __stdin;
FILE __stderr;







int fputc(int c, FILE *f) {
	while(!USART_GetFlagStatus(USART1,USART_FLAG_TC)) {};
	USART_SendData(USART1, c);
	while(1) {
		if (USART_GetFlagStatus(USART1, USART_FLAG_TC)) //transfer is completed
			return c;
	}
	return ferror (f);
}

void _ttywrch(int ch) {
	while(!USART_GetFlagStatus(USART1,USART_FLAG_TXE)) {};
	USART_SendData(USART1, ch);

	
}

int fgetc(FILE *f) {
	while(!(USART_GetFlagStatus(USART1, USART_FLAG_RXNE))) {}
	int c = USART_ReceiveData(USART1);
	USART_SendData(USART1, c);
	while(!(USART_GetFlagStatus(USART1, USART_FLAG_TC))) {}
	return c;
}

int ferror(FILE *f) {
	// Your implementation of ferror 
	return EOF;
}

// this function is called after main returns. That shall never happen
void _sys_exit(int return_code) {
	while(1) {
		__BKPT(0xAA);
	}
}       


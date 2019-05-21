#include "mcu_support_package/inc/stm32f10x.h"

#include <stdio.h>

int a;

void init(void) {
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_USART1, ENABLE);
	

	GPIO_InitTypeDef Rx;
	Rx.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	Rx.GPIO_Pin = GPIO_Pin_10;
	Rx.GPIO_Speed = GPIO_Speed_10MHz;
	GPIO_Init(GPIOA, &Rx);
	
	GPIO_InitTypeDef Tx;
	Tx.GPIO_Mode = GPIO_Mode_AF_PP;
	Tx.GPIO_Pin = GPIO_Pin_9;
	Tx.GPIO_Speed = GPIO_Speed_10MHz;
	GPIO_Init(GPIOA, &Tx);
	

	USART_InitTypeDef usart1;
	
	usart1.USART_BaudRate = 9600;
	usart1.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	usart1.USART_Mode = (USART_Mode_Rx | USART_Mode_Tx);
	usart1.USART_Parity = USART_Parity_No;
	usart1.USART_StopBits = USART_StopBits_1;
	usart1.USART_WordLength = USART_WordLength_8b;
	USART_Init(USART1, &usart1);
	USART_Cmd(USART1, ENABLE);

}

int main(void) {
float d = 3.2;
float b;
	init();
	printf("%f \n", d);
	printf("%f \n", d);
	scanf("%e", &d );
	scanf("%d%f", &a, &b);
	printf("D is equal to %f", d);
	printf("B is equal to %f", b);
	while(1);

	return 0; 
}








// В Project->Options->Linker, Scatter File выбран файл stack_protection.sct
// он обеспечивает падение в HardFault при переполнении стека
// Из-за этого может выдаваться ложное предупреждение "AppData\Local\Temp\p2830-2(34): warning:  #1-D: last line of file ends without a newline"

#ifdef USE_FULL_ASSERT

// эта функция вызывается, если assert_param обнаружил ошибку
void assert_failed(uint8_t * file, uint32_t line) {
	/* User can add his own implementation to report the file name and line number,
	ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */

	(void)file;
	(void)line;

	__disable_irq();
	while(1) {
		// это ассемблерная инструкция "отладчик, стой тут"
		// если вы попали сюда, значит вы ошиблись в параметрах вызова функции из SPL.
		// Смотрите в call stack, чтобы найти ее
		__BKPT(0xAB);
	}
}

#endif

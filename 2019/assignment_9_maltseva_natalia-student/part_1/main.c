#include "mcu_support_package/inc/stm32f10x.h"


void SendCharUSART1(char ch);
char GetCharUSART1(void);

int main(void) {
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_USART1, ENABLE);
	
	//приемник
	GPIO_InitTypeDef Rx;
	Rx.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	Rx.GPIO_Pin = GPIO_Pin_10;
	Rx.GPIO_Speed = GPIO_Speed_10MHz;
	GPIO_Init(GPIOA, &Rx);
	
	//передатчик
	GPIO_InitTypeDef Tx;
	Tx.GPIO_Mode = GPIO_Mode_AF_PP;
	Tx.GPIO_Pin = GPIO_Pin_9;
	Tx.GPIO_Speed = GPIO_Speed_10MHz;
	GPIO_Init(GPIOA, &Tx);
	
	//настройка UART	
	USART_InitTypeDef usart1;
	
	usart1.USART_BaudRate = 9600;
	usart1.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	usart1.USART_Mode = (USART_Mode_Rx | USART_Mode_Tx);
	usart1.USART_Parity = USART_Parity_No;
	usart1.USART_StopBits = USART_StopBits_1;
	usart1.USART_WordLength = USART_WordLength_8b;
	USART_Init(USART1, &usart1);
	USART_Cmd(USART1, ENABLE);
	
	//кнопка на PA0
	GPIO_InitTypeDef button;
	button.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	button.GPIO_Pin = GPIO_Pin_0;
	button.GPIO_Speed = GPIO_Speed_10MHz;
	
	//Светодиод PC8
	GPIO_InitTypeDef led;
	led.GPIO_Mode = GPIO_Mode_Out_PP;
	led.GPIO_Pin = GPIO_Pin_8;
	led.GPIO_Speed = GPIO_Speed_10MHz;
	
	GPIO_Init(GPIOA, &button);
	GPIO_Init(GPIOC, &led);
	
	uint16_t received_message = 0;
	
	while(1) {
		if (USART_GetFlagStatus(USART1, USART_FLAG_RXNE)) { //получаем сообщение, если не 0, то зажигаем светодиод
			received_message = USART_ReceiveData(USART1);
			if (received_message) 
				GPIO_SetBits(GPIOC,GPIO_Pin_8);  
			else GPIO_ResetBits(GPIOC,GPIO_Pin_8);  
		}
		
		if ((USART_GetFlagStatus(USART1, USART_FLAG_TXE)) || (USART_GetFlagStatus(USART1, USART_FLAG_TC))) { //отправляем 1 при нажатой кнопке, 0 при отжатой
			if (GPIOA->IDR & (1<<0)) {
				USART_SendData(USART1, 1);
			}	else USART_SendData(USART1, 0);
			
		}
		
	}
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
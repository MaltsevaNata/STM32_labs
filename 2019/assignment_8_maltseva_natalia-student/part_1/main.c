#include "mcu_support_package/inc/stm32f10x.h"
#include <stdbool.h>

static volatile unsigned int loops = 0;
static volatile bool flag = true; //определяет, когда заполнение дошло до максимума и нужно вести отсчет в обратную сторону

void TIM3_IRQHandler(void)
{
	if (flag) //увеличиваем яркость
		loops++;
	else loops --;
	if (loops >= 100 || loops == 0) //инверсия,уменьшаем яркость
		flag = !flag;
	
	TIM_SetCompare3(TIM3, loops); //подаем напряжение

	TIM_ClearFlag(TIM3, TIM_FLAG_Update);

}


int main(void)
{
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE); //светодиода PC8
	
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);	//TIM3

	RCC_APB2PeriphClockCmd( RCC_APB2Periph_AFIO, ENABLE ); //тактирование внутреннего мультиплексора

	TIM_TimeBaseInitTypeDef Timer_3;


	TIM_TimeBaseStructInit(&Timer_3); //заполнение по умолчанию

	Timer_3.TIM_Prescaler = SystemCoreClock / 10000 - 1; //задаем нужную частоту 10 кГц
	//каждые 10^-4 новая ступенька
	Timer_3.TIM_Period = 100; //через 100 ступенек происходит прерывание, то есть 10^-2 с
	//чтобы светодиод зажегся за 1 секунду полностью, нужно 100 таких периодов

	TIM_TimeBaseInit(TIM3, &Timer_3);

	GPIO_InitTypeDef led;
	led.GPIO_Mode = GPIO_Mode_AF_PP;

	GPIO_PinRemapConfig(GPIO_FullRemap_TIM3, ENABLE); 
	led.GPIO_Pin = GPIO_Pin_8;
	led.GPIO_Speed = GPIO_Speed_10MHz;
	GPIO_Init(GPIOC, &led);
	
	TIM_ITConfig(TIM3, TIM_IT_Update, ENABLE);
	TIM_Cmd(TIM3, ENABLE);
	NVIC_EnableIRQ(TIM3_IRQn);

	TIM_OCInitTypeDef tim_channel;
	TIM_OCStructInit(&tim_channel); //заполнение по умолчанию
	tim_channel.TIM_OCMode = TIM_OCMode_PWM1;
	tim_channel.TIM_OutputState = TIM_OutputState_Enable; 
	TIM_OC3Init(TIM3, &tim_channel);
	
	while(1);
	


	return 0;
}








// В Project->Options->Linker, Scatter File выбран файл stack_protection.sct
// он обеспечивает падение в HardFault при переполнении стека
// Из-за этого может выдаваться ложное предупреждение "AppData\Local\Temp\p2830-2(34): warning:  #1-D: last line of file ends without a newline"

#ifdef USE_FULL_ASSERT

// эта функция вызывается, если assert_param обнаружил ошибку
void assert_failed(uint8_t * file, uint32_t line)
{
	/* User can add his own implementation to report the file name and line number,
	ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */

	(void)file;
	(void)line;

	__disable_irq();
	while(1)
	{
		// это ассемблерная инструкция "отладчик, стой тут"
		// если вы попали сюда, значит вы ошиблись в параметрах вызова функции из SPL.
		// Смотрите в call stack, чтобы найти ее
		__BKPT(0xAB);
	}
}

#endif

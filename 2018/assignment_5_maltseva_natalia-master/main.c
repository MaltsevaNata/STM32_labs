#include "mcu_support_package/inc/stm32f10x.h"

#define ASSIGNMENT_PART_1_BUTTON_LED    1
#define ASSIGNMENT_PART_2_LED_BLINKING  2
#define ASSIGNMENT_PART_3_THREE_BUTTONS 3
#define ASSIGNMENT_CURRENT_PART 3

#if ASSIGNMENT_CURRENT_PART == ASSIGNMENT_PART_1_BUTTON_LED

int main(void)
{
	RCC->APB2ENR |= 1<<2; //clock enable
	RCC->APB2ENR |= 1<<4;
	
	GPIOA->CRL &= ~(0xF);
	GPIOC->CRH &= ~(0xF);
	
	GPIOA->CRL |= (1<<2); //mode input floating for button
	GPIOC->CRH |= (1<<0);//mode output push pull for led
		
	while (1)
	{
		if (GPIOA->IDR & (1<<0))
			GPIOC->ODR |= 1<<8;  //the button is pressed
		else GPIOC->ODR &= ~(1<<8);
	}
    
}

#elif ASSIGNMENT_CURRENT_PART == ASSIGNMENT_PART_2_LED_BLINKING

int main(void)
{
	RCC->APB2ENR |= 1<<4; //clock enable
	
	GPIOC->CRH &= ~(0xFF); 
	GPIOC->CRH |= (1<<0) | (1<<4); //mode output push pull
	
	GPIOC->ODR |= 1<<8; //initial led states
	GPIOC->ODR &= ~(1<<9); 
	
	__disable_irq();
	SysTick_Config(SystemCoreClock/50);	//blink rate 50 Hz (0,02000004 s)
	
	while(1) {
		if (SysTick->CTRL & (1<<16) ) { 
			GPIOC->ODR ^= 1<<8; //change led states
			GPIOC->ODR ^= 1<<9;
		}
	}
}

#elif ASSIGNMENT_CURRENT_PART == ASSIGNMENT_PART_3_THREE_BUTTONS

int main(void)
{
	RCC->APB2ENR |= 1<<2; //clock enable for leds
	RCC->APB2ENR |= 1<<3; //clock enable for buttons
	
	GPIOA->CRL &= ~(0xFFFFFFF0);
	GPIOB->CRL &= ~(0xFFF);
	
	GPIOA->CRL |= (1<<4) | (1<<8)| (1<<12) | (1<<16) | (1<<20) | (1<<24) | (1<<28);//mode output push pull for leds
	GPIOB->CRL |= (1<<3) | (1<<7) | (1<<11); //mode input floating for buttons	

	while (1) {
		GPIOA->ODR  |= ((1<<(GPIOB->IDR+1))&(~(1<<8)));
		GPIOA->ODR &= ~(0xFF-((1<<(GPIOB->IDR+1))&(~(1<<8)))); 
	}
}

#else

    #error "You should define ASSIGNMENT_CURRENT_PART to enable some part of the assignment"

#endif

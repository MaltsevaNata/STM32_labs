#include "mcu_support_package/inc/stm32f10x.h"

#define ASSIGNMENT_PART_1_SLOW_BLINKING  1
#define ASSIGNMENT_PART_2_DEBOUNCER      2
#define ASSIGNMENT_PART_3_NINE_LEDS      3
#define ASSIGNMENT_PART_4_SOMETHING_COOL 4

#define ASSIGNMENT_CURRENT_PART 3

#if ASSIGNMENT_CURRENT_PART == ASSIGNMENT_PART_1_SLOW_BLINKING

volatile static int loops=0;

void SysTick_Handler(void)
{
	loops++;
	if (loops==10) { //blink rate 2s
		GPIOC->ODR ^= 1<<9; //change led states
		loops=0;
	}	
}

int main(void)
{
	RCC->APB2ENR |= 1<<4; //clock enable
	GPIOC->CRH &= ~(0xF0); //clean bits for PC9
	GPIOC->CRH |= (1<<4);//mode output push pull for led
	__disable_irq();
	SysTick_Config(SystemCoreClock/5);	//rate 0.2 s
	__enable_irq();
	while(1) {}
}

#elif ASSIGNMENT_CURRENT_PART == ASSIGNMENT_PART_2_DEBOUNCER

volatile static int pr_but = 0;
volatile static int unpr_but = 0;

void SysTick_Handler(void)
{
	if (GPIOA->IDR & (1<<0)) { //the button is pressed
		unpr_but=0;
		pr_but++; 												
		if (pr_but==10) { //blink rate 2s
			GPIOC->ODR |= 1<<8;  
			pr_but=0;
		}
	}
	
	else { //the button is unpressed 
		pr_but = 0;
		unpr_but++;
		if (unpr_but==15) { //blink rate 3s
			GPIOC->ODR &= ~(1<<8);  
			unpr_but=0;
		}
	}
}


int main(void)
{
	RCC->APB2ENR |= 1<<2; //clock enable
	RCC->APB2ENR |= 1<<4;
	
	GPIOA->CRL &= ~(0xF); //clean bits for PC8
	GPIOC->CRH &= ~(0xF); //clean bits for PA0
	
	GPIOA->CRL |= (1<<2); //mode input floating for button
	GPIOC->CRH |= (1<<0);//mode output push pull for led
	
	__disable_irq();
	SysTick_Config(SystemCoreClock/5);	//rate 0.2 s
	__enable_irq();
	while (1)
	{}
}

#elif ASSIGNMENT_CURRENT_PART == ASSIGNMENT_PART_3_NINE_LEDS

volatile static int loops[9] = {0, 0, 0, 0, 0, 0, 0, 0, 0};
volatile static int rates[9];
void SysTick_Handler(void)
{
	for (int i=0; i<9;i++) {
		loops[i]++;
		if (loops[i]==rates[i]) { //blink rate 2s
			GPIOC->ODR ^= 1<<(i+1); //change led states
			loops[i]=0;
		}
	}
}

int main(void)
{
	const int blink_rate[9] = {1000,100,2000,4000,8000,10000,12000,14000,16000}; // ms
	for (int i=0; i<9; i++) {
		rates[i] = blink_rate[i]/100; //values to count loops
	}
	
	RCC->APB2ENR |= 1<<2; //clock enable for leds
	
	GPIOA->CRL &= ~(0xFFFFFFF0); //clean bits for PC1-PC7
	GPIOA->CRH &= ~(0xFF); //clean bits for PC8-PC9
	
	GPIOA->CRL |= (1<<4) | (1<<8)| (1<<12) | (1<<16) | (1<<20) | (1<<24) | (1<<28);//mode output push pull for leds PC1-PC7
	GPIOA->CRH |= (1<<0) | (1<<4); //mode output push pull for leds PC8-PC9
	
	__disable_irq();
	SysTick_Config(SystemCoreClock/5);	//rate 0.2 s
	__enable_irq();
	
	while (1)
	{}
	
}

#elif ASSIGNMENT_CURRENT_PART == ASSIGNMENT_PART_4_SOMETHING_COOL

int main(void)
{
    return 0;
}

#else

    #error "You should define ASSIGNMENT_CURRENT_PART to enable some part of the assignment"

#endif

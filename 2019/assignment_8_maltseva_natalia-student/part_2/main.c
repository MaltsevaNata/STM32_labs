#include "mcu_support_package/inc/stm32f10x.h"
#include <stdbool.h>

static void scan(void);
static void sinus(void);
static const int sins[180] = {100,    103,    107,    110,    114,    117,    121,    124,    128,    131,    134,    137,    141,    144,    147,
                              150,    153,    156,    159,    162,    164,    167,    169,    172,    174,    177,    179,    181,    183,    185,
                              187,    188,    190,    191,    193,    194,    195,    196,    197,    198,    198,    199,    199,    200,    200,
                              200,    200,    200,    199,    199,    198,    198,    197,    196,    195,    194,    193,    191,    190,    188,
                              187,    185,    183,    181,    179,    177,    174,    172,    169,    167,    164,    162,    159,    156,    153,
                              150,    147,    144,    141,    137,    134,    131,    128,    124,    121,    117,    114,    110,    107,    103,
                              100,     97,     93,     90,     86,     83,     79,     76,     72,     69,     66,     63,     59,     56,     53,
                              50,     47,     44,     41,     38,     36,     33,     31,     28,     26,     23,     21,     19,     17,     15,
                              13,     12,     10,      9,      7,      6,      5,      4,      3,      2,      2,      1,      1,      0,      0,
                              0,      0,      0,      1,      1,      2,      2,      3,      4,      5,      6,      7,      9,     10,     12,
                              13,     15,     17,     19,     21,     23,     26,     28,     31,     33,     36,     38,     41,     44,     47,
                              50,     53,     56,     59,     63,     66,     69,     72,     76,     79,     83,     86,     90,     93,     97
                             };

static int piano[8] = {0}; //массив кнопок нажаты - 1, не нажаты - 0 
static unsigned int sum[8] = {0}; //массив фаз синусов
static int summa = 0; //фаза суммарного синуса
static bool sound = 0;
static int inds[8] = {0}; //индексы массива синуса для каждой синусоиды
static int step[8]; //шаги приращения синусоид

void sinus(void) {
	int n = 0;
	for (int but =0; but <8; but ++) {
		if (piano[but] == 1) {
			if (inds[but] <0) inds[but]=0;
			sum[but] = sins[inds[but]/1000]; //избавляемся от дробной части делением на 1000
			summa+=sum[but];
			n++;
		} else 
			sum[but] = 0;
	}
	summa /= n;
}


void increase_index(int ind) {
	inds[ind] += step[ind];
	if(inds[ind] >= 180000) inds[ind] -= 180000; //индексы массива синусов от 0 до 180 (домножены на 1000)

}

void TIM3_IRQHandler(void) {	
	for (int i = 0; i<9; i++) {
		increase_index(i);
	}

	TIM_ClearFlag(TIM3, TIM_FLAG_Update);

}


int main(void) {
	const int freq_diskr = 51428;
	const int freq[8] = {1046, 1175, 1318, 1396, 1568, 1720, 1975, 2093};
  
	//Сенсорные кнопки РА1..РА7
	GPIO_InitTypeDef buttons;	
	GPIO_InitTypeDef buttonPC4;
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE); //динамик PC6 и кнопка PC4
	buttonPC4.GPIO_Mode = GPIO_Mode_IPD;
	buttons.GPIO_Mode = GPIO_Mode_IPD;
	buttons.GPIO_Pin = GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_3 | GPIO_Pin_4 | GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7;
	buttonPC4.GPIO_Pin = GPIO_Pin_4;
	GPIO_Init(GPIOA, &buttons);
	GPIO_Init(GPIOC, &buttonPC4);	
	
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);	//TIM3

	RCC_APB2PeriphClockCmd( RCC_APB2Periph_AFIO, ENABLE ); //тактирование внутреннего мультиплексора

	TIM_TimeBaseInitTypeDef Timer_3;


	TIM_TimeBaseStructInit(&Timer_3); //заполнение по умолчанию

	Timer_3.TIM_Prescaler = SystemCoreClock / 10000000 - 1; 

	Timer_3.TIM_Period = 200; 

	TIM_TimeBaseInit(TIM3, &Timer_3);

	GPIO_InitTypeDef speaker;
	speaker.GPIO_Mode = GPIO_Mode_AF_PP;

	GPIO_PinRemapConfig(GPIO_FullRemap_TIM3, ENABLE); 
	speaker.GPIO_Pin = GPIO_Pin_6;
	speaker.GPIO_Speed = GPIO_Speed_10MHz;
	GPIO_Init(GPIOC, &speaker);
	
	TIM_ITConfig(TIM3, TIM_IT_Update, ENABLE);
	TIM_Cmd(TIM3, ENABLE);
	
	
	//настройка канала 1
	TIM_OCInitTypeDef tim_channel;
	TIM_OCStructInit(&tim_channel); //заполнение по умолчанию
	tim_channel.TIM_OCMode = TIM_OCMode_PWM1;
	tim_channel.TIM_OutputState = TIM_OutputState_Enable; 
	TIM_OC1Init(TIM3, &tim_channel);
	
	
	__enable_irq();
	NVIC_EnableIRQ(TIM3_IRQn);
	for (int but = 0; but<8; but++) {
		step[but] = freq[but]*180*1000/freq_diskr; //дополнительно храним 3 знака после запятой
	}
	while(1) {
		sound = 0;
		summa = 0;
		scan();
		sinus();
		if (sound) TIM_SetCompare1(TIM3, summa); 
		else TIM_SetCompare1(TIM3, 0);
	}
	
}


void scan() { 
	__enable_irq();
	for (int but = 0; but <7; but++) { //кнопки РА1..РА7
		if (GPIOA->IDR & (1<<(but+1))) {
			piano[but] = 1;
			sound = 1;
		} else {
			piano[but] = 0;
		}
	}
	if (GPIOC->IDR & (1<<(4))) { //кнопка PC4
		piano[7] = 1;
		sound = 1;
	} else {
		piano[7] = 0;
	}
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

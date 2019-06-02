#include "mcu_support_package/inc/stm32f10x.h"
#include "functions.h"

#define BUFFER_SIZE 30 //но если буфер 100, то памяти уже не хватает :(

static volatile int16_t tim_period = 500;
static volatile int8_t play_enable = 0;

static int32_t filter(int32_t * voltage);
static int32_t get_frequency(int32_t dist);
static void read_distance(int32_t * voltage, int16_t counter);
static void check_counter(int16_t* counter);
static void tune_sound(int32_t distance);

void init (void) {

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOC, ENABLE);
	
	// DAC_OUT
	GPIO_InitTypeDef input;
	input.GPIO_Mode = GPIO_Mode_AIN;
	input.GPIO_Pin = GPIO_Pin_4;
	input.GPIO_Speed = GPIO_Speed_10MHz;
	GPIO_Init(GPIOC, &input);
	
	// COMP/Trig
	GPIO_InitTypeDef output;
	output.GPIO_Mode = GPIO_Mode_Out_PP;
	output.GPIO_Pin = GPIO_Pin_1;
	output.GPIO_Speed = GPIO_Speed_10MHz;
	GPIO_Init(GPIOA, &output);
	
	//динамик
	GPIO_InitTypeDef speaker;
	speaker.GPIO_Mode = GPIO_Mode_Out_PP;
	speaker.GPIO_Pin = GPIO_Pin_6;
	speaker.GPIO_Speed = GPIO_Speed_10MHz;
	GPIO_Init(GPIOC, &speaker);
	
	//ADC
	ADC_InitTypeDef ADC_InitStructure;
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);
	ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;
	ADC_InitStructure.ADC_ScanConvMode = DISABLE;
	ADC_InitStructure.ADC_ContinuousConvMode = ENABLE; 
	ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;
	ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
	ADC_InitStructure.ADC_NbrOfChannel = 1;
	
	ADC_RegularChannelConfig(ADC1, ADC_Channel_14, 1, ADC_SampleTime_28Cycles5); //Т1
	ADC_Init ( ADC1, &ADC_InitStructure);   
	
	//Калибровка АЦП
	
	// Включаем АЦП
	ADC_Cmd( ADC1, ENABLE );
	// Сбрасываем калибровку
	ADC_ResetCalibration( ADC1 );
	// Ждем окончания сброса
	while( ADC_GetResetCalibrationStatus( ADC1 ) ) {;}
	// Запускаем калибровку
	ADC_StartCalibration( ADC1 );
	// Ждем окончания калибровки
	while( ADC_GetCalibrationStatus( ADC1 ) ) {;}
	
	//Запуск измерения
	ADC_SoftwareStartConvCmd(ADC1, ENABLE);
	
	SysTick_Config(SystemCoreClock/1000); //начальная частота тиков таймера (10 см)
}

int main(void) {
	static volatile int32_t distance = 0;
	static int32_t voltage [BUFFER_SIZE] = {0}; //данные с датчика
	static int16_t counter = 0; //счетчик для буфера
	
	init(); 
	
	while(1) {
		check_counter(&counter); //проверяем, не переполнился ли массив с данными
		read_distance((int32_t *)voltage, counter); //получаем 1 значение с датчика
		distance = filter((int32_t *) voltage);	//фильтруем и преобразуем в расстояние
		tune_sound(distance); //настраиваем частоту звука 
	}

	return 0; 
}

void check_counter(int16_t* counter) {
	if (*counter >= BUFFER_SIZE) { //если буфер заполнен, начинаем заполнение заново
		*counter = 0;
	} else {
		*counter+=1;
	}
}


void read_distance(int32_t * voltage, int16_t counter) {

	GPIO_SetBits(GPIOA, GPIO_Pin_1); //высокий уровень на impulse

	while ((ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC)==0)) {;}; //Проверка флага готовности сэмпла 
	
	
	voltage[counter] = ADC_GetConversionValue(ADC1);	
	
	GPIO_ResetBits(GPIOA, GPIO_Pin_1); //низкий уровень на impulse

	

}

static volatile int8_t flag = 1; //для четных и нечетных тиков таймера

void SysTick_Handler(void) {
	if (flag) {
		GPIO_SetBits(GPIOC, GPIO_Pin_6); //высокий уровень
	} else {
		GPIO_ResetBits(GPIOC, GPIO_Pin_6);  //низкий уровень
	}
	flag = !flag;

}

int32_t filter(int32_t * voltage) {
	int32_t dist;  
	dist = getMedian((int32_t *)voltage, BUFFER_SIZE); //усреднение значений
	dist = ((dist - 23.11) /4.98); //преобразование в см
	return dist - (dist%5); //округление на 5
}


int32_t get_frequency(int32_t dist) {
	return dist*(-11)+1110; //превращение расстояния в частоту
}

void tune_sound(int32_t distance) {
	if (distance >= 250) {
		__disable_irq();
		return;
	}
	static volatile int32_t freq = 0;
	static volatile int32_t temp_freq = 0;
	temp_freq = freq;
	freq = get_frequency(distance);
	if (temp_freq != freq) {	//если частота с прошлого измерения изменилась, перенастраиваем звук
		__disable_irq();
		SysTick_Config(SystemCoreClock/(2*freq)); 
		__enable_irq();
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

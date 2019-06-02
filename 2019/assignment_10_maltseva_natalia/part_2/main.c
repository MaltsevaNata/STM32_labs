#include "mcu_support_package/inc/stm32f10x.h"
#include <stdio.h>
#include "../spl/inc/stm32f10x_dma.h"
#include "functions.h"
#include "math.h"

#define BUFFER_SIZE 30
#define PI 3.1415926 
#define G 1125 //подобрано экспериментально

static void calibrate(uint32_t * offset_voltage);
static void get_voltage(uint32_t * received_voltage);
static void wait_for_calibration(void);
static void convert_to_angles(uint32_t * received_voltage, int32_t* angle_X,  int32_t* angle_Y, uint32_t* offset_voltage);
static void turn_light(int32_t *angle_X,  int32_t *angle_Y);

static volatile uint16_t ADC_Result[3];

void init(void) {
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOC | RCC_APB2Periph_USART1, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);

	//ось Х
	GPIO_InitTypeDef X;
	X.GPIO_Mode = GPIO_Mode_AIN;
	X.GPIO_Pin = GPIO_Pin_1;
	X.GPIO_Speed = GPIO_Speed_10MHz;
	GPIO_Init(GPIOA, &X);

	//ось Y
	GPIO_InitTypeDef Y;
	Y.GPIO_Mode = GPIO_Mode_AIN;
	Y.GPIO_Pin = GPIO_Pin_2;
	Y.GPIO_Speed = GPIO_Speed_10MHz;
	GPIO_Init(GPIOA, &Y);

	//ось Z
	GPIO_InitTypeDef Z;
	Z.GPIO_Mode = GPIO_Mode_AIN;
	Z.GPIO_Pin = GPIO_Pin_3;
	Z.GPIO_Speed = GPIO_Speed_10MHz;
	GPIO_Init(GPIOA, &Z);
	
	//Светодиод +X
	GPIO_InitTypeDef plus_X;
	plus_X.GPIO_Mode = GPIO_Mode_Out_PP;
	plus_X.GPIO_Pin = GPIO_Pin_2;
	plus_X.GPIO_Speed = GPIO_Speed_10MHz;
	GPIO_Init(GPIOC, &plus_X);
	
	//Светодиод -X
	GPIO_InitTypeDef minus_X;
	minus_X.GPIO_Mode = GPIO_Mode_Out_PP;
	minus_X.GPIO_Pin = GPIO_Pin_0;
	minus_X.GPIO_Speed = GPIO_Speed_10MHz;
	GPIO_Init(GPIOC, &minus_X);
	
	//Светодиод +Y
	GPIO_InitTypeDef plus_Y;
	plus_Y.GPIO_Mode = GPIO_Mode_Out_PP;
	plus_Y.GPIO_Pin = GPIO_Pin_3;
	plus_Y.GPIO_Speed = GPIO_Speed_10MHz;
	GPIO_Init(GPIOC, &plus_Y);
	
	//Светодиод -Y
	GPIO_InitTypeDef minus_Y;
	minus_Y.GPIO_Mode = GPIO_Mode_Out_PP;
	minus_Y.GPIO_Pin = GPIO_Pin_1;
	minus_Y.GPIO_Speed = GPIO_Speed_10MHz;
	GPIO_Init(GPIOC, &minus_Y);
	
	//кнопка на PA0 для начала и окончания калибровки
	GPIO_InitTypeDef button;
	button.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	button.GPIO_Pin = GPIO_Pin_0;
	button.GPIO_Speed = GPIO_Speed_10MHz;	
	GPIO_Init(GPIOA, &button);
	
	//USART для общения с пользователем через перенаправление printf
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
	
	
	//DMA
	DMA_DeInit(DMA1_Channel1);
	DMA_InitTypeDef DMA_InitStructure;
	// Данные из регистра данных ADC1
	DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)&(ADC1->DR);
	// Переправляем данные в массив ADC_Result
	DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)&ADC_Result[0];
	// Передача данных из периферии в память
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;
	// Размер буфера
	DMA_InitStructure.DMA_BufferSize = 3;
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
	// Настройки размера данных
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;
	DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;
	DMA_InitStructure.DMA_Priority = DMA_Priority_High;
	DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
	DMA_Init(DMA1_Channel1, &DMA_InitStructure);
	// Включаем первый канал DMA1
	DMA_Cmd(DMA1_Channel1, ENABLE);
	// Включаем работу ДМА через АЦП
	ADC_DMACmd(ADC1, ENABLE);

	//ADC
	ADC_InitTypeDef ADC_InitStructure;
	ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;
	ADC_InitStructure.ADC_ScanConvMode = ENABLE;
	ADC_InitStructure.ADC_ContinuousConvMode = ENABLE; 
	ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;
	ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
	ADC_InitStructure.ADC_NbrOfChannel = 3;
	ADC_RegularChannelConfig(ADC1, ADC_Channel_1, 1, ADC_SampleTime_28Cycles5); //PA1
	ADC_RegularChannelConfig(ADC1, ADC_Channel_2, 2, ADC_SampleTime_28Cycles5); //PA2
	ADC_RegularChannelConfig(ADC1, ADC_Channel_3, 3, ADC_SampleTime_28Cycles5); //PA3
	ADC_Init ( ADC1, &ADC_InitStructure);            
	
	// Включаем АЦП
	ADC_Cmd( ADC1, ENABLE );
	
	//Калибровка АЦП
	
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

}


int main(void) {
	static volatile uint32_t offset_voltage[3] = {0}; //массив для калибровки в нулевом положении
	static volatile uint32_t received_voltage[3] = {0};
	static int32_t angle_X = 0;
	static int32_t angle_Y = 0;
	
	init();
	wait_for_calibration();
	calibrate((uint32_t *) offset_voltage);
	while(1) {
		get_voltage((uint32_t *) received_voltage);
		convert_to_angles((uint32_t *) received_voltage, &angle_X, &angle_Y, (uint32_t*) offset_voltage );
		turn_light(&angle_X, &angle_Y);
	}
	return 0; 
}

void wait_for_calibration(void) {
	printf("Put the sensor on the table \nand press and release the button PA0 on the board \nto start calibration.");
	while ((GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_0))==0) {};
	while ((GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_0))==1) {};
	return;
}


void calibrate(uint32_t * offset_voltage) {
	printf("\nThe calibration has started. \nWait until the calibration is finished");
	get_voltage(offset_voltage);
	offset_voltage[2]-=G;
	printf("\nThe calibration completed. Accelerometer is ready");
}

void get_voltage(uint32_t * voltage) {
	volatile uint32_t x[BUFFER_SIZE]; 
	volatile uint32_t y[BUFFER_SIZE]; 
	volatile uint32_t z[BUFFER_SIZE]; 
	volatile uint32_t counter;
	for (counter = 0; counter < BUFFER_SIZE; counter++) {
		x[counter] = ADC_Result[0];
		y[counter] = ADC_Result[1];
		z[counter] = ADC_Result[2];
	}
	//усреднение значений
	voltage[0] = getMedian((int32_t *)x, counter); 
	voltage[1] = getMedian((int32_t *)y, counter); 
	voltage[2] = getMedian((int32_t *)z, counter); 
}

void convert_to_angles(uint32_t * received_voltage, int32_t* angle_X,  int32_t* angle_Y, uint32_t* offset_voltage) {
	volatile double delta_xyz[3];
	for (volatile int32_t counter = 0; counter<3; counter++) {
		delta_xyz[counter] = (double)received_voltage[counter]-(double)offset_voltage[counter];
	}
	*angle_X = (int32_t)((atan(delta_xyz[0]/fabs(delta_xyz[2])))* 180.0 / PI)*2;
	*angle_Y = (int32_t)((atan(delta_xyz[1]/fabs(delta_xyz[2])))* 180.0 / PI)*2;
}


void turn_light(int32_t* angle_X,  int32_t* angle_Y) {
	if ( (*angle_X > -20) && (*angle_X < 20)) {
		GPIO_ResetBits(GPIOC,GPIO_Pin_2);
		GPIO_ResetBits(GPIOC,GPIO_Pin_0); 
	}	else if (*angle_X >= 20) {
		GPIO_SetBits(GPIOC,GPIO_Pin_0); 
		GPIO_ResetBits(GPIOC,GPIO_Pin_2);		
	} else {
		GPIO_SetBits(GPIOC,GPIO_Pin_2);  
		GPIO_ResetBits(GPIOC,GPIO_Pin_0);
	}
	if ((*angle_Y > -20) && (*angle_Y < 20)) {
		GPIO_ResetBits(GPIOC,GPIO_Pin_1);
		GPIO_ResetBits(GPIOC,GPIO_Pin_3);
	} else if (*angle_Y >= 20) {
		GPIO_SetBits(GPIOC,GPIO_Pin_1);  
		GPIO_ResetBits(GPIOC,GPIO_Pin_3);
	}	else {
		GPIO_SetBits(GPIOC,GPIO_Pin_3);
		GPIO_ResetBits(GPIOC,GPIO_Pin_1);
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

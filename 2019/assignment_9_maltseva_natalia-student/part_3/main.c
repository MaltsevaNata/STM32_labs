#include "mcu_support_package/inc/stm32f10x.h"

static uint8_t check_title_B (void);
static uint8_t check_title_C (void);
static void send_answer(void);
//static uint8_t check_sum(uint8_t b1,uint8_t b2, uint8_t b3, uint8_t b4); //для проверки контрольной суммы
static void get_freq(void);
static void play(void);
static uint8_t crc(uint8_t * message, uint8_t length);

static uint8_t play_enable = 0;
static uint8_t count = 0; //количество правильных запросов
static uint8_t summa = 0; //сумма данных в запросе для сравнения с checksum
static uint8_t crc_got[4]; //массив данных сообщения для проверки
static uint8_t byte = 0; //текущий принимаемый байт
static uint8_t next_byte = 0; //флаг о новом байте
static uint8_t check_passed = 0; //если все условия выполнены и можно отправлять ответ
static uint64_t click_counter = 1; // флаг для подачи и снятия напряжения на динамике
static 	uint16_t frequency = 0;

void USART2_IRQHandler(void) {
	byte = USART_ReceiveData(USART2);
	next_byte = 1; //оповещение о принятии байта
	USART_ClearFlag(USART2, USART_FLAG_RXNE);
}

void SysTick_Handler(void) {
	if (play_enable) {
		if (!(click_counter%2)) { //если четный щелчок, подаем напряжение, нечетный - убираем
			GPIO_SetBits(GPIOA, GPIO_Pin_6);
			click_counter--;
		}	else {
			GPIO_ResetBits(GPIOA, GPIO_Pin_6);
			click_counter ++;
		}
	}
}

int main(void) {
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE );
	RCC_APB1PeriphClockCmd ( RCC_APB1Periph_USART2, ENABLE);
	

	GPIO_InitTypeDef Rx;
	Rx.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	Rx.GPIO_Pin = GPIO_Pin_3;
	Rx.GPIO_Speed = GPIO_Speed_10MHz;
	GPIO_Init(GPIOA, &Rx);
	
	GPIO_InitTypeDef Tx;
	Tx.GPIO_Mode = GPIO_Mode_AF_PP;
	Tx.GPIO_Pin = GPIO_Pin_2;
	Tx.GPIO_Speed = GPIO_Speed_10MHz;
	GPIO_Init(GPIOA, &Tx);
	

	USART_InitTypeDef usart2;
	
	usart2.USART_BaudRate = 57600;
	usart2.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	usart2.USART_Mode = (USART_Mode_Rx | USART_Mode_Tx);
	usart2.USART_Parity = USART_Parity_No;
	usart2.USART_StopBits = USART_StopBits_1;
	usart2.USART_WordLength = USART_WordLength_8b;
	USART_Init(USART2, &usart2);
	USART_Cmd(USART2, ENABLE);
	NVIC_EnableIRQ(USART2_IRQn);
	USART_ITConfig(USART2, USART_IT_RXNE, ENABLE); //прерывание по событию - принятию байта
	
	GPIO_InitTypeDef GPIO_speaker;
	GPIO_speaker.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_speaker.GPIO_Pin = GPIO_Pin_6;
	GPIO_speaker.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_speaker);
	
	//проверка CRC8
	/*uint8_t stroka[9] = "123456789"; 
	uint8_t crc8 = crc(stroka, 9);*/
	
	while(1) {
		check_passed = 0;
		frequency = 0;
		summa = 0;
		if (check_title_B()) //проверка заголовка и получение частоты
			continue;
		uint8_t checksum = byte;
		if (summa != checksum) // проверка контрольной суммы или CRC
			continue;
		check_passed = 1;
		count++;
		play();
		send_answer();
	}

}



uint8_t check_title_B (void) {
	uint8_t title_check = 0xBB; 
	if (byte == title_check) { //проверяем соответствие первых 2х байт заголовку 
		//summa += byte; //проверка контрольной суммой
		crc_got[0] = byte; //проверка CRC
		while (!next_byte) {}; //ждем принятия любого нового байта
		next_byte = 0;
		if (check_title_C()) return 1;
		return 0;
	}
	return 1;
}

uint8_t check_title_C (void) {
	
	uint8_t title_check = 0xCC;
	if (byte == title_check) { //проверяем соответствие первых 2х байт заголовку 
		//summa += byte;  //проверка контрольной суммой
		crc_got[1] = byte; //проверка CRC
		while (!next_byte) {}; //ждем принятия любого нового байта
		next_byte = 0;
		get_freq();
		return 0;
	}
	return 1;
}


void send_answer(void) {
	while  (!USART_GetFlagStatus(USART2, USART_FLAG_TXE)) {;} 
	USART_SendData(USART2, 0xAA); 
	while  (!USART_GetFlagStatus(USART2, USART_FLAG_TXE)) {;} 
	USART_SendData(USART2, 0xCC);
	while  (!USART_GetFlagStatus(USART2, USART_FLAG_TXE)) {;} 
	USART_SendData(USART2, count);
	while  (!USART_GetFlagStatus(USART2, USART_FLAG_TXE)) {;} 
	
	uint8_t crc_reply[3]; //массив данных ответа для проверки
	crc_reply[0] = 0xAA;
	crc_reply[1] = 0xCC;
	crc_reply[2] = count;
	
	USART_SendData(USART2, crc(crc_reply, 3)); //с проверкой CRC
	//USART_SendData(USART2, check_sum(0xAA, 0xCC, count, 0)); с контрольной суммой
	
}


void get_freq(void) {
	//summa += byte;  //проверка контрольной суммой
	crc_got[2] = byte; //проверка CRC
	while (!next_byte) {}; //ждем принятия любого нового байта
	next_byte = 0;
	//summa += byte;  //проверка контрольной суммой
	crc_got[3] = byte; //проверка CRC
	frequency = ((crc_got[3]<<8) + crc_got[2]);
	//summa = summa%256;  //проверка контрольной суммой
	summa = crc(crc_got, 4); //проверка CRC
	while (!next_byte) {}; //ждем принятия любого нового байта
	next_byte = 0;
	return;
}


/*uint8_t check_sum(uint8_t b1, uint8_t b2, uint8_t b3, uint8_t b4) { //для проверки контрольной суммы
	return (b1+b2+b3+b4)%256;
}*/


void play() {
	if (frequency == 0) { //выключаем звук
		play_enable = 0;
		GPIO_ResetBits(GPIOA, GPIO_Pin_6);
		return;
	}
	play_enable = 1;
	__disable_irq();
	SysTick_Config(SystemCoreClock/(frequency*2)); //включаем звук
	__enable_irq();
}


/*Poly  : 0x31    x^8 + x^5 + x^4 + 1
Init  : 0xFF
Revert: false
XorOut: 0x00
Check : 0xF7 ("123456789") */

uint8_t crc(uint8_t * message, uint8_t length) {
	uint8_t polynom = 0x31;
	uint8_t remainder;	//остаток от деления, в нем же останется результат
	remainder = 0xFF; 
	for (uint8_t bytes = 0; bytes < length; bytes ++) {
		remainder = remainder ^ *message ++; //XOR со всеми элементами сообщения по очереди
		for (uint8_t bit = 0; bit <8; bit++)	{
			if (remainder & 0x80) { //старший бит остатка 1
				remainder = (remainder << 1 ) ^ polynom; // XOR остаток и полином, получаем новый остаток
			}		else {
				remainder = remainder << 1; // Сдвигаем остаток на 1 бит влево, помещая в конец новый бит исходных данных
			}
		}
	}		
	return (remainder);
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


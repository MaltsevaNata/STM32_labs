#include "mcu_support_package/inc/stm32f10x.h"

#define PART_1_SPEAKER  1
#define PART_2_MATRIX   2

#define CURRENT_PART 1

void scan();
void play();
void led_tune();
void light();

volatile static int but[3][4] = {0};
volatile static int freq[3][4] = {{554, 622, 739, 830}, {932, 1108, 1244, 1480}, {1661, 1864, 2217, 2489}};

volatile static int matrix[5][7] = {0};

void SysTick_Handler(void) {
}

void scan() {
	__disable_irq();

	SysTick_Config(SystemCoreClock/10000);

	__enable_irq();
	for (int col = 1; col<4; col++) {
		GPIOC->ODR &= ~(1<<col); //подаем 0 на один столбец
		while (!(SysTick->CTRL & (1<<16) )) {}
		for (int row = 1; row <5; row++) {

			if (!(GPIOA->IDR & (1<<row))) { //если на строке 0, то нажата кнопка, в соответствующую ячейку массива заносим 1
				but[col-1][row-1] = 1;

			}	else but[col-1][row-1] = 0;
		}

		GPIOC->ODR |= (1<<col); //возвращаем 1 на проверенный столбец

	}
}



#if CURRENT_PART == PART_1_SPEAKER

void play() {
	for (int col = 0; col<=3; col++) {
		for (int row = 0; row <=4; row++) {

			if (but[col][row] == 1) { //проверяем все кнопки
				__disable_irq();

				SysTick_Config(SystemCoreClock/freq[col][row]); //настраиваем частоту звучания

				__enable_irq();
				while (!(SysTick->CTRL & (1<<16) )) {}
				GPIO_SetBits(GPIOB, GPIO_Pin_10); //подаем напряжение на динамик 

				while (!(SysTick->CTRL & (1<<16) )) {}
				GPIO_ResetBits(GPIOB, GPIO_Pin_10); //убираем напряжение с динамика

			}	else {
				GPIO_ResetBits(GPIOB, GPIO_Pin_10); //если кнопка не нажата, то звук выключен

			}

		}
	}
}

int main(void) {
	GPIO_InitTypeDef GPIO_cols; //создаем экземпляры структур
	GPIO_InitTypeDef GPIO_rows;
	GPIO_InitTypeDef GPIO_speaker;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE); //подаем тактирование на порты
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);

	GPIO_cols.GPIO_Mode = GPIO_Mode_Out_OD; //заполняем поля структур
	GPIO_rows.GPIO_Mode = GPIO_Mode_IPU; //вход, подтянутый к питанию
	GPIO_speaker.GPIO_Mode = GPIO_Mode_Out_PP;

	GPIO_cols.GPIO_Pin = GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_3;

	GPIO_rows.GPIO_Pin = GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_3 | GPIO_Pin_4;


	GPIO_speaker.GPIO_Pin = GPIO_Pin_10;

	GPIO_cols.GPIO_Speed = GPIO_Speed_10MHz;
	GPIO_rows.GPIO_Speed = GPIO_Speed_10MHz;
	GPIO_speaker.GPIO_Speed = GPIO_Speed_50MHz;

	GPIO_Init(GPIOC, &GPIO_cols); //применяем настройки
	GPIO_Init(GPIOA, &GPIO_rows);
	GPIO_Init(GPIOB, &GPIO_speaker);

	__disable_irq();

	SysTick_Config(SystemCoreClock/10000);

	__enable_irq();

	for (int col = 1; col<4; col++) //на все столбцы 1
		GPIOC->ODR |= (1<<col);
	while(1) {
		scan();
		play();
	}
}


#elif CURRENT_PART == PART_2_MATRIX

int main(void) {
	GPIO_InitTypeDef GPIO_cols;
	GPIO_InitTypeDef GPIO_rows;
	GPIO_InitTypeDef GPIO_matrix_cols;
	GPIO_InitTypeDef GPIO_matrix_rows;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);

	GPIO_cols.GPIO_Mode = GPIO_Mode_Out_OD;
	GPIO_rows.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_matrix_cols.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_matrix_rows.GPIO_Mode = GPIO_Mode_Out_PP;

	GPIO_cols.GPIO_Pin = GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_3;

	GPIO_rows.GPIO_Pin = GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_3 | GPIO_Pin_4;


	GPIO_matrix_cols.GPIO_Pin = GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7 | GPIO_Pin_8 | GPIO_Pin_9;
	GPIO_matrix_rows.GPIO_Pin = GPIO_Pin_9 | GPIO_Pin_10 | GPIO_Pin_11 | GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15;

	GPIO_cols.GPIO_Speed = GPIO_Speed_10MHz;
	GPIO_rows.GPIO_Speed = GPIO_Speed_10MHz;
	GPIO_matrix_cols.GPIO_Speed = GPIO_Speed_10MHz;
	GPIO_matrix_rows.GPIO_Speed = GPIO_Speed_10MHz;

	GPIO_Init(GPIOC, &GPIO_cols);
	GPIO_Init(GPIOA, &GPIO_rows);
	GPIO_Init(GPIOA, &GPIO_matrix_cols);
	GPIO_Init(GPIOB, &GPIO_matrix_rows);

	__disable_irq();

	SysTick_Config(SystemCoreClock/10000);

	__enable_irq();

	for (int col = 1; col<4; col++) //на все столбцы клавиатуры подаем напряжение
		GPIOC->ODR |= (1<<col);

	for (int matrix_col = 5; matrix_col <10; matrix_col++) //на столбцы матрицы 0
		GPIOA->ODR &= ~(1<<matrix_col);

	for (int matrix_row = 9; matrix_row <16; matrix_row++) //на строки матрицы 1 -> выключаем все
		GPIOB->ODR |= (1<<matrix_row);

	while(1) {
		scan();
		led_tune();
		light();
	}
}



void led_tune() { //для каждой нажатой кнопки настраиваем, какие диоды будут светиться
	int col;
	int row;
	for (col = 0; col<=3; col++) {
		for ( row = 0; row <=4; row++) {

			if (but[col][row] == 1) {
				if (col == 0 && row == 0) { //1

					for (int matrix_col = 0; matrix_col<=4; matrix_col++) {
						for (int matrix_row = 0; matrix_row<=6; matrix_row++) {
							if (matrix_col == 4) {
								matrix[4][matrix_row] = 1;
								continue;
							}	else {
								if ((matrix_col == 3 || matrix_col == 2) && matrix_row == 0) {
									matrix[matrix_col][matrix_row] = 1;
									continue;
								} else matrix[matrix_col][matrix_row] = 0;
							}
						}
					}
					return;
				}


				if (col == 1 && row == 0) {	 //2
					for (int matrix_col = 0; matrix_col<=4; matrix_col++) {
						for (int matrix_row = 0; matrix_row<=6; matrix_row++) {
							if (matrix_row == 0 || matrix_row == 3 || matrix_row == 6) {
								matrix[matrix_col][matrix_row] = 1;
								continue;
							}	else {
								if (matrix_col == 4 && (matrix_row == 1 || matrix_row == 2)) {
									matrix[4][matrix_row] = 1;
									continue;
								}

								else {
									if ((matrix_col ==0) && (matrix_row == 4 || matrix_row == 5 )) {
										matrix[matrix_col][matrix_row] = 1;
										continue;
									} else matrix[matrix_col][matrix_row]=0;
								}
							}
						}
					}
					return;
				}

				if (col == 2 && row ==0) { //3
					for (int matrix_col = 0; matrix_col<=4; matrix_col++) {
						for (int matrix_row = 0; matrix_row<=6; matrix_row++) {
							if (matrix_row == 0 || matrix_row == 3 || matrix_row == 6) {
								matrix[matrix_col][matrix_row] = 1;
								continue;
							}	else {
								if (matrix_col == 4) {
									matrix[4][matrix_row] = 1;
									continue;
								}	else matrix[matrix_col][matrix_row]=0;

							}

						}
					}
					return;
				}

				if (col == 0 && row ==1) { //4
					for (int matrix_col = 0; matrix_col<=4; matrix_col++) {
						for (int matrix_row = 0; matrix_row<=6; matrix_row++) {
							if (matrix_row == 3) {
								matrix[matrix_col][matrix_row] = 1;
								continue;
							} else {
								if (matrix_col == 0 && (matrix_row == 0 || matrix_row == 1 || matrix_row == 2)) {
									matrix[matrix_col][matrix_row] = 1;
									continue;
								}	else {
									if (matrix_col == 4) {
										matrix[4][matrix_row] = 1;
										continue;
									}	else matrix[matrix_col][matrix_row]=0;
								}
							}

						}
					}
					return;
				}

				if (col == 1 && row == 1) {	 //5
					for (int matrix_col = 0; matrix_col<=4; matrix_col++) {
						for (int matrix_row = 0; matrix_row<=6; matrix_row++) {
							if (matrix_row == 0 || matrix_row == 3 || matrix_row == 6) {
								matrix[matrix_col][matrix_row] = 1;
								continue;
							}	else {
								if (matrix_col == 0 && (matrix_row == 1 || matrix_row == 2)) {
									matrix[matrix_col][matrix_row] = 1;
									continue;
								}	else {
									if ((matrix_col ==4) && (matrix_row == 4 || matrix_row == 5 )) {
										matrix[matrix_col][matrix_row] = 1;
										continue;
									}	else matrix[matrix_col][matrix_row]=0;
								}
							}
						}
					}
					return;
				}

				if (col == 2 && row == 1) {	 //6
					for (int matrix_col = 0; matrix_col<=4; matrix_col++) {
						for (int matrix_row = 0; matrix_row<=6; matrix_row++) {
							if (matrix_row == 0 || matrix_row == 3 || matrix_row == 6) {
								matrix[matrix_col][matrix_row] = 1;
								continue;
							}	else {
								if (matrix_col == 0 ) {
									matrix[matrix_col][matrix_row] = 1;
									continue;
								}

								else {
									if ((matrix_col ==4) && (matrix_row == 4 || matrix_row == 5 )) {
										matrix[matrix_col][matrix_row] = 1;
										continue;
									}	else matrix[matrix_col][matrix_row]=0;
								}
							}
						}
					}
					return;
				}


				if (col == 0 && row == 2) { //7
					for (int matrix_col = 0; matrix_col<=4; matrix_col++) {
						for (int matrix_row = 0; matrix_row<=6; matrix_row++) {
							if (matrix_col == 4) {
								matrix[4][matrix_row] = 1;
								continue;
							}	else {
								if (matrix_row == 0 || matrix_row == 3) {
									matrix[matrix_col][matrix_row] = 1;
									continue;
								}

								else matrix[matrix_col][matrix_row] = 0;
							}
						}
					}
					return;
				}


				if (col == 1 && row == 2) {	 //8
					for (int matrix_col = 0; matrix_col<=4; matrix_col++) {
						for (int matrix_row = 0; matrix_row<=6; matrix_row++) {
							if (matrix_row == 0 || matrix_row == 3 || matrix_row == 6) {
								matrix[matrix_col][matrix_row] = 1;
								continue;
							}	else {
								if (matrix_col == 0 ) {
									matrix[matrix_col][matrix_row] = 1;
									continue;
								}	else {
									if (matrix_col ==4) {
										matrix[matrix_col][matrix_row] = 1;
										continue;
									} else matrix[matrix_col][matrix_row]=0;
								}
							}
						}
					}
					return;
				}


				if (col == 2 && row == 2) {	 //9
					for (int matrix_col = 0; matrix_col<=4; matrix_col++) {
						for (int matrix_row = 0; matrix_row<=6; matrix_row++) {
							if (matrix_row == 0 || matrix_row == 3 || matrix_row == 6) {
								matrix[matrix_col][matrix_row] = 1;
								continue;
							}	else {
								if (matrix_col == 4 ) {
									matrix[matrix_col][matrix_row] = 1;
									continue;
								}	else {
									if ((matrix_col ==0) && (matrix_row == 1 || matrix_row == 2 )) {
										matrix[matrix_col][matrix_row] = 1;
										continue;
									}	else matrix[matrix_col][matrix_row]=0;
								}
							}
						}
					}
					return;
				}


				if (col == 0 && row == 3) { //*
					for (int matrix_col = 0; matrix_col<=4; matrix_col++) {
						for (int matrix_row = 0; matrix_row<=6; matrix_row++) {
							if (matrix_col == 2) {
								matrix[matrix_col][matrix_row] = 1;
								continue;
							}	else {
								if (matrix_row == 3) {
									matrix[matrix_col][matrix_row] = 1;
									continue;
								} else {
									if (matrix_row == matrix_col + 1) {
										matrix[matrix_col][matrix_row] = 1;
										continue;
									}	else {
										if ((matrix_row == 1 && matrix_col == 4) || (matrix_row == 2 && matrix_col == 3) ||	(matrix_row == 4 && matrix_col == 1) || (matrix_row ==5 && matrix_col ==0)) {
											matrix[matrix_col][matrix_row] = 1;
											continue;
										}	else matrix[matrix_col][matrix_row] = 0;
									}
								}


							}
						}
					}
					return;
				}


				if (col == 1 && row ==3) { //0
					for (int matrix_col = 0; matrix_col<=4; matrix_col++) {
						for (int matrix_row = 0; matrix_row<=6; matrix_row++) {
							if (matrix_row == 0 || matrix_row == 6) {
								matrix[matrix_col][matrix_row] = 1;
								continue;
							}	else {
								if (matrix_col == 4 || matrix_col == 0) {
									matrix[matrix_col][matrix_row] = 1;
									continue;
								}	else matrix[matrix_col][matrix_row]=0;

							}

						}
					}
					return;
				}


				if (col == 2 && row ==3) { //#
					for (int matrix_col = 0; matrix_col<=4; matrix_col++) {
						for (int matrix_row = 0; matrix_row<=6; matrix_row++) {
							if (matrix_row == 2 || matrix_row == 4) {
								matrix[matrix_col][matrix_row] = 1;
								continue;
							}	else {
								if (matrix_col == 1 || matrix_col == 3) {
									matrix[matrix_col][matrix_row] = 1;
									continue;
								}	else matrix[matrix_col][matrix_row]=0;

							}

						}
					}
					return;
				}
			}

		}

	}
	for (int matrix_col = 0; matrix_col<=4; matrix_col++) {
		for (int matrix_row = 0; matrix_row<=6; matrix_row++) {
			matrix[matrix_col][matrix_row] = 0;
		}
	}
}

void light() {
	__disable_irq();

	SysTick_Config(SystemCoreClock/10000);	//rate 0.1 ms

	__enable_irq();

	for (int matrix_cols = 0; matrix_cols < 5; matrix_cols++) {
		for (int matrix_rows = 0; matrix_rows < 7; matrix_rows++) {
			if (matrix[matrix_cols] [matrix_rows] == 1) { //включаем нужный диод
				while (!(SysTick->CTRL & (1<<16) )) {}
				GPIOB->ODR &= ~( 1 << (9 + matrix_rows)); //на строку 0
				GPIOA->ODR |= ( 1 << (5 + matrix_cols)); //на столбец 1
				while (!(SysTick->CTRL & (1<<16) )) {}
				GPIOB->ODR |=  1 << (9 + matrix_rows); //выключаем
				GPIOA->ODR &= ~( 1 << (5 + matrix_cols));
			}	else {
				GPIOB->ODR |=  1 << (9 + matrix_rows); //выключаем
				GPIOA->ODR &= ~( 1 << (5 + matrix_cols));
			}
		}
	}
}


#else

#error "You should define CURRENT_PART to enable some part of the assignment"

#endif

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

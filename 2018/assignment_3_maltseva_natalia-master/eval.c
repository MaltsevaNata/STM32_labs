#include "mcu_support_package/inc/stm32f10x.h"

#include "main/eval.h"
#include <string.h>
#include <stdlib.h>

static int counter;
static int64_t count_1 = 0; //first number in the braces
static int64_t count_2 = 0; //second number
static int64_t count = 0; //subtotal
static char myformula [60]; //changable copy of the formula 
static char chislo_1[60]; //stores the current number
static char *ptr; //static char check;
static int level[60]; //array of procedure tracks

char number (char symbol) 
	{ 
		if ((symbol >= '0') && (symbol <= '9'))
			return 1;
		else return 0;
}
	
char znak (char symbol)
{
	if ((symbol == '+') || (symbol == '-') || (symbol == '*') ||
			(symbol == '/') )
		return 1;
	else return 0;
}

int32_t eval( const char * formula )
{
	strcpy(myformula, formula);
	unsigned int length = 0;
	int64_t result = 0;
	int32_t answer = 0;
	int drob = 0;
	int lev_drob = 0;
	int max_level = 0;
	int min_level = 15;
	int lev = 1 ; //number of procedure track for the symbol
	for (int i = 0; i < strlen(myformula); ++i)
	{
			if (myformula[i] == ' ') {                  //clean all spaces
					for (int k = i; k < strlen(myformula); k++) 
							myformula[k] = myformula[k + 1];
			}
			if (myformula[i] == ' ')
					i--;
	}
	while ((myformula[length] == ')') || znak(myformula[length]) || (myformula[length] == '(') || (number(myformula[length])))
			length ++;
	myformula[length] = ('\0');
	for (int i = 0; i < length ; ++i) { 		//procedure track
			if ((myformula[i] == '(') || (number(myformula[i]))) 
			{
					if (!(number(myformula[i-1]))) lev++; 
					level[i] = lev;
					if (level[i] > max_level) max_level = level[i];
			}
			else {
					lev--;
					level[i] = lev;
				if (level[i] < min_level) min_level = level[i];
			}
	}
	do 	{
		char sign = ' ';        //sign between numbers
		char out_sign = ' ';    // sign outside braces
		int sym = 0;
		char minus_flag = 0;
		char found = 0;
		while (!found) {				//go to the current procedure 
				if (level[sym] == max_level) 
				{
					if (number(myformula[sym]))
					{
							found = 1; 
							break;
					}
					else sym++;
				}
				if (sym > length)
				{
					if (max_level == min_level) 
					{
						 if (drob != 0)
							result += drob/100;
						 return answer = (int32_t)result;
					 }
					else 
					{
						max_level --;
						sym = 0;
					}
			}
			else sym++;
		}
		counter = 0;
		for (int prev_sym = 	1; prev_sym <= 4; prev_sym++) //searching for previous sign
		{
			if (prev_sym==1) 
				{
					if (myformula[sym - prev_sym] == '-') 
						minus_flag = 1;
					if (((myformula[sym-1] == '+') || (myformula[sym-1] == '*') ||
															(myformula[sym-1] == '/')) && ((myformula[sym-2] == '(')||(myformula[sym-2] == ')')))
						out_sign = myformula[sym-1];
				}
			if (prev_sym==4) {
				if ((znak(myformula[sym-prev_sym])) && ((myformula[sym-prev_sym+1] == '(')&&(myformula[sym-prev_sym+2] == '(')))
						out_sign = myformula[sym-prev_sym];}
			else 
					if ((znak(myformula[sym-prev_sym])) && ((myformula[sym-prev_sym+1] == '(')||(myformula[sym-prev_sym+2] == '(')))
						out_sign = myformula[sym-prev_sym];
		}
			while (number(myformula[sym])) //recording string number
				{
				chislo_1[counter] = myformula[sym];
				level[sym] = 60;
				counter++;
				sym++;
				}
			chislo_1[counter] = '\0';
			count_1 = strtoll(chislo_1,  &ptr, 10);   //type conversion to int 
			if (minus_flag)
					count_1 *= (-1);
			if ((znak(myformula[sym])) && (number(myformula[sym+1]))) //if there's one more number in the braces
			{
					max_level = level[sym];
					sign = myformula[sym];
					sym++;
					counter = 0;
					while (number(myformula[sym])) {
							chislo_1[counter] = myformula[sym];
							level[sym] = 60;
							counter++;
							sym++;
					}
			chislo_1[counter] = '\0';
			count_2 = strtoll(chislo_1,  &ptr, 10);
			switch (sign) {															//procedure inside braces
					case '-':
							count = count_1 - count_2;
							break;
					case '+':
							count = count_1 + count_2;
							break;
					case '*':
							count = count_1 * count_2;
							break;
					case '/':
							drob = (count_1 % count_2* 100 / count_2);
							lev_drob = max_level;
							count= count_1/count_2;
							break;
				}
			}
			else 
					count = count_1;
			switch (out_sign) { //procedure outside braces
							case '-':
									result -= count;
									break;
							case '+':
									result +=  count;
									break;
							case '*':
									result *= count;
									if (lev_drob == 4 )
									{
											drob *= count;
										lev_drob = 0;
									}
									break;
							case '/':
									result /= count;
									break;
							default:
								result += count;
								break;
					}
				} while (max_level >= min_level);
		answer = (int32_t)result;
    return answer;
}





int64_t eval_64( const char * formula )
{
		return eval (formula);
}

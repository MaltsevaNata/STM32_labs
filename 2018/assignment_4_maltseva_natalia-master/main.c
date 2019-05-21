#include "mcu_support_package/inc/stm32f10x.h"
#include "functions.h"

int main() {
	int32_t a[10]={ 15, 21, 4, 2, 1, 78, 2, 35, 45, 11 };
	sort((int32_t *)a, 10);
	int32_t b[13]={ 20, 15, 3, 21, 4, 15, 1, 78, 2, 35, 76, 45, 11 };
	int32_t res = getOrderStatistic((int32_t *)b, 13, 1);
	int32_t med = getMedian((int32_t *)b, 13);
	ArrayOf11 str= { 15, 21, 4, 2, 1, 78, 2, 35, 45, 11, 6  };
	str = sortStruct( str );
	return 0;
}

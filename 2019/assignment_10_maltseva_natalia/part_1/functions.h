#include <stdint.h>

void sort( int32_t * arr, uint32_t size );

int32_t getMedian( int32_t * arr, uint32_t size ); 

int32_t getOrderStatistic( int32_t * arr, uint32_t size, uint32_t j );

typedef struct {
	int32_t a[11];
} ArrayOf11;

ArrayOf11 sortStruct( ArrayOf11 array );


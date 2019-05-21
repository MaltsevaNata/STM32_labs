#include <stdint.h>
int8_t Aint8;
int16_t Bint16;
int32_t Cint32;
int64_t Dint64;

uint8_t Euint8;
uint16_t Fuint16;
uint32_t Guint32;
uint64_t Huint64;

int8_t  Arint[5];
uint8_t  Aruint[5];

float Ifloat;
double Jdouble;
int K;
int *ukaz;

int main(void)
{
	Aint8 = 200;
	Bint16 = -35;
	Cint32 = 217088;
	Dint64 = 18964;
	Euint8 = 55;
	Fuint16 = 5;
	Guint32 = 68;
	Ifloat = 6.7;
	Jdouble = 9.9;
	
	ukaz = &K;
	*ukaz = 8;
	
	
	Arint[0] = 0;
	Arint[1] = 1;
	Arint[2] = 2;
	
  return 0;
}
#include "mcu_support_package/inc/stm32f10x.h"
#include "main/eval.h"
#include "main/bitmagic.h"

#include <stdint.h>
#include <stdlib.h>
#include <limits.h>

#define STRINGIFY( x ) #x

#define TEST_EQUAL( value, expected )                                   \
{                                                                       \
																																				\
    int32_t v = value;                                                  \
    if( v != expected )                                                 \
    {                                                                   \
         return 2;       																							  \
																																				\
    }                                                                   \
}                                                                       \

int main(void)
{

		TEST_EQUAL( eval("1+2"),  3  );

		TEST_EQUAL( eval("1"),    1  );

		TEST_EQUAL( eval("5*5"),  25 );

		TEST_EQUAL( eval("10/3"), 3  );

		TEST_EQUAL( eval("3-2"),  1  );

		// tests from the assignment
    TEST_EQUAL( eval("(1+2)"),          3);

    TEST_EQUAL( eval("((0*2)-14)"),   -14);

    TEST_EQUAL( eval("(100 - (9/1))"), 91);

    TEST_EQUAL( eval("1+(-1)"),         0);


    // having fun around zero
    TEST_EQUAL( eval("(0)"),    0 );

    TEST_EQUAL( eval("(0-1)"), -1 );

    TEST_EQUAL( eval("(0-1)"), -1 );

    TEST_EQUAL( eval("(1*0)"),  0 );

    TEST_EQUAL( eval("(-1)"),       -1 );

    TEST_EQUAL( eval("((-1)*(-1))"), 1 );

    TEST_EQUAL( eval("( 1 * 0) "),  0 );
		
		// more realistic examples
    TEST_EQUAL( eval("(((10/3)*5)-10)+(9*3)"), 33  );
    TEST_EQUAL( eval("3-2"),  1  );

    // corner cases :3
    //TEST_EQUAL( eval(" "),  0 );
    TEST_EQUAL( eval("((((100/1)*1)/1)*1)/1"), 100 );
	
    // result will fit in the int32, but I didn't say anything about temproraries
    TEST_EQUAL( eval("(2147483647 - 1)"),  INT_MAX - 1 );
    TEST_EQUAL( eval("(2147483647*2)/2"), INT_MAX );

    TEST_EQUAL( eval("((-1)*((-1)*(-2147483648)))"), INT_MIN );
    TEST_EQUAL( eval("(((-2)*(-2147483648)))/(-2)"), INT_MIN );

    TEST_EQUAL( eval("(-(-(-(-1))))"), 1 );
    TEST_EQUAL( eval("(((((((((((((((((((((((((((((10)))))))))))))))))))))))))))))"), 10 );
		
		
		TEST_EQUAL( countSetBits(0xF), 4 );
    TEST_EQUAL( countSetBits(0),   0 );
    TEST_EQUAL( countSetBits(1),   1 );
    
    TEST_EQUAL( countSetBits( 0xF << 7 ),  4 );
    TEST_EQUAL( countSetBits(0xFFFFFFFF), 32 );
    TEST_EQUAL( countSetBits(0x80000000),  1 );
    TEST_EQUAL( countSetBits(0xAAAAAAAA), 16 );
    
    TEST_EQUAL( countLeadingZeros(5), 29 );
    TEST_EQUAL( countLeadingZeros(0), 32 );
    TEST_EQUAL( countLeadingZeros(1), 31 );
    TEST_EQUAL( countLeadingZeros(7), 29 );
    TEST_EQUAL( countLeadingZeros(3), 30 );
    
    TEST_EQUAL( countLeadingZeros(1<<7),  32-7-1 );
    TEST_EQUAL( countLeadingZeros(1<<10), 32-10-1 );
    TEST_EQUAL( countLeadingZeros(1<<20), 32-20-1 );
    TEST_EQUAL( countLeadingZeros(1<<30), 32-30-1 );
    TEST_EQUAL( countLeadingZeros(1<<1),  32-1-1 );
    
    TEST_EQUAL( countLeadingZeros(0xAAAAAAAA), 0 );
    TEST_EQUAL( countLeadingZeros(0xFFFFFFFF), 0 );

	return 0;
		
}



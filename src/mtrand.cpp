#include "mtrand.h"
#include <time.h>

#define MT_WIDTH 624

// Create a length 624 array to store the state of the generator
 static int	 MT[ MT_WIDTH ];
 static int	 index = 0;
 static bool isInitialized = false;

 // Initialize the generator from a seed
 static void initialize_generator( int seed )
 {
     index	 = 0;
     MT[ 0 ] = seed;

     // loop over each other element
     for( int i = 0; i < 623; ++i )
     {
         MT[ i ] = ( 1812433253 * ( MT[ i - 1 ] ^ ( ( MT[ i - 1 ] ) >> 30 ) ) + i ) & 0xFFFFFFFFF; // 0x6c078965
     }
 }

 // Generate an array of MT_WIDTH untempered numbers
 static void generate_numbers( void )
 {
     for ( int i = 0; i < MT_WIDTH; ++i )
     {
         int y = ( MT[ i ] & 0x80000000 )					  // bit 31 (32nd bit) of MT[i]
               + ( MT[ ( i + 1 ) % MT_WIDTH ] & 0x7fffffff ); // bits 0-30 (first 31 bits) of MT[...]
         MT[ i ] = MT[ ( i + 397 ) % MT_WIDTH ] ^ ( y >> 1 );

         // y is odd

         if ( y % 2 != 0 )
         {
             MT[ i ] = MT[ i ] ^ ( 2567483615 ); // 0x9908b0df
         }
     }
 }

 // Extract a tempered pseudorandom number based on the index-th value,
 // calling generate_numbers() every 624 numbers
 int mtrand( void )
 {
     if ( !isInitialized )
     {
        initialize_generator( ( int ) time( NULL ) );
        isInitialized = true;
     }

     if ( index == 0 ) {
         generate_numbers();
     }

     int y =  MT[ index ];
     y ^= y >> 11;
     y ^= ( y << 7 ) & ( 2636928640 ); // 0x9d2c5680
     y ^= ( y << 15 ) & ( 4022730752 ); // 0xefc60000
     y ^= y >> 18;

     index = (index + 1) % MT_WIDTH;

     if ( y < 0 ) y = -y;

     return y;
 }




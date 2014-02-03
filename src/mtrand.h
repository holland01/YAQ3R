#pragma once

/*
======================================

Mersenne Twister algorithm, transposed to C.

More info at http://en.wikipedia.org/wiki/Mersenne_twister

======================================
*/

int mtrand();

inline int mtrand_range( int min, int max ) { return mtrand() % max + min; }

#ifndef COMMON_H
#define COMMON_H

/*
Commonly used definitions.
*/

//#define _TRACE 1
//#define _COMPARE_ALGORITHMS 1

// Ensure cross platform compatibility of std::min and std::max:
#define MIN(a,b) ((a) < (b) ? (a) : (b))
#define MAX(a,b) ((b) < (a) ? (a) : (b))

#define EPSILON 1e-6

#endif // COMMON_H
// Lookup table based sin/cos using degrees.
//
// MUST call initialize_fast_math() before using the fast_ functions.

#ifndef FAST_MATH_H
#define FAST_MATH_H

#include <cmath>
#include <cstdio>
#include <cassert>

//#define FAST_MATH_ASSERT(WHAT) assert(WHAT)
#define FAST_MATH_ASSERT(WHAT)

// wtf this should be defined
# define M_PI		3.14159265358979323846	/* pi */

#define DEGREES_TO_RADIANS(ANGLE) (-((ANGLE) + 90) * (M_PI / 180.0f))

#define LOOKUPS_PER_DEGREE 10

// Enough for values from 0..360 inclusive + repeat first 90 degrees for cosine
#define NUM_LOOKUP_VALUES ((360+90) * LOOKUPS_PER_DEGREE)
#define LOOKUPS_PER_CIRCLE (360 * LOOKUPS_PER_DEGREE)

#define LOOKUP_PRECISION (1.0f / LOOKUPS_PER_DEGREE)

extern float sin_lookup[NUM_LOOKUP_VALUES];
extern float* cos_lookup;

void initialize_fast_math();

// direct lookup of the table, val has to between 0 and NUM_LOOKUP_VALUES
inline float fast_lookup_sin(size_t val) { FAST_MATH_ASSERT(val < NUM_LOOKUP_VALUES); return sin_lookup[val]; }
inline float fast_lookup_cos(size_t val) { FAST_MATH_ASSERT(val < NUM_LOOKUP_VALUES); return cos_lookup[val]; }

#endif // FAST_MATH_H

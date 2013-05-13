#include "fast_math.hpp"

float sin_lookup[NUM_LOOKUP_VALUES];
float* cos_lookup;

//
void initialize_fast_math()
{
    for(int i = 0; i < NUM_LOOKUP_VALUES; i++)
    {
        float angle = (float)(i%(360*LOOKUPS_PER_DEGREE)) / LOOKUPS_PER_DEGREE;
        sin_lookup[i] = sin(DEGREES_TO_RADIANS(angle + LOOKUP_PRECISION));
    }

    // Ensure the cardinal directions are 100% accurate.
    sin_lookup[0 * LOOKUPS_PER_DEGREE] = -1;
    sin_lookup[90 * LOOKUPS_PER_DEGREE] = 0;
    sin_lookup[180 * LOOKUPS_PER_DEGREE] = 1;
    sin_lookup[270 * LOOKUPS_PER_DEGREE] = 0;
    sin_lookup[360 * LOOKUPS_PER_DEGREE] = -1;
    cos_lookup = sin_lookup + 90*LOOKUPS_PER_DEGREE;
}

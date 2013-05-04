#include "fast_math.hpp"

float sin_lookup[NUM_LOOKUP_VALUES];

//
void initialize_fast_math()
{
    for(int i = 0; i < NUM_LOOKUP_VALUES; i++)
    {
        float angle = (float)i / LOOKUPS_PER_DEGREE;
        sin_lookup[i] = sin(DEGREES_TO_RADIANS(angle + LOOKUP_PRECISION));
    }

    // Ensure the cardinal directions are 100% accurate.
    sin_lookup[0 * LOOKUPS_PER_DEGREE] = 0;
    sin_lookup[90 * LOOKUPS_PER_DEGREE] = 1;
    sin_lookup[180 * LOOKUPS_PER_DEGREE] = 0;
    sin_lookup[270 * LOOKUPS_PER_DEGREE] = -1;
}

float fast_sin_deg(float degrees)
{
    int index = (int)(degrees * LOOKUPS_PER_DEGREE);
    while (index < 0) index += NUM_LOOKUP_VALUES;
    while (index > NUM_LOOKUP_VALUES) index -= NUM_LOOKUP_VALUES;

    return sin_lookup[index];
}

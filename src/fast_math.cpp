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
    for (int i = 0; i <= 270; i += 90)
    {
        sin_lookup[i * LOOKUPS_PER_DEGREE] = sin(DEGREES_TO_RADIANS(i));
    }
}

float fast_sin_deg(float degrees)
{
    int index = (int)(degrees * LOOKUPS_PER_DEGREE);
    while (index < 0) index += NUM_LOOKUP_VALUES;
    while (index > NUM_LOOKUP_VALUES) index -= NUM_LOOKUP_VALUES;

    return sin_lookup[index];
}

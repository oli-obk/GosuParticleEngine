#include "Particle.hpp"
#include "fast_math.hpp"

//static_assert(std::numeric_limits<decltype(Particle::angle)>::max() > NUM_LOOKUP_VALUES*2, "Particle::angle doesn't have enough bits for the lookup table and safe rotations");

void Particle::update()
{
    // Apply friction
    velocity_x *= 1.0 - friction;
    velocity_y *= 1.0 - friction;

    // Gravity.
    velocity_y += /*gravity*/ 0.0;

    // Move
    x += velocity_x;
    y += velocity_y;

    // Rotate.
    angle += angular_velocity;
    if (angle >= LOOKUPS_PER_CIRCLE) {
        angle -= LOOKUPS_PER_CIRCLE;
    }

    // Resize.
    scale += zoom;

    // Fade out.
    color.alpha -= (fade / 255.0);

    time_to_live--;

    // Die if out of time, invisible or shrunk to nothing.
    if((color.alpha <= 0) ||
            (scale <= 0))
    {
        time_to_live = 0;
    }
}

Particle Particle::Angle(float gosu_degrees) const
{
    Particle p = *this;
    p.angle = gosu_degrees * LOOKUPS_PER_DEGREE;
    return p;
}

Particle Particle::AngularVelocity(float gosu_degrees_per_second) const
{
    int r = gosu_degrees_per_second * LOOKUPS_PER_DEGREE;
    while (r >= LOOKUPS_PER_CIRCLE) {
        r -= LOOKUPS_PER_CIRCLE;
    }
    // make sure it is positive, rotating by 359 degrees is the same as -1
    while (r < 0) {
        r += LOOKUPS_PER_CIRCLE;
    }
    Particle p = *this;
    p.angular_velocity = r;
    return p;
}

Particle Particle::TimeToLive(uint16_t frames) const
{
    Particle p = *this;
    p.time_to_live = frames;
    return p;
}

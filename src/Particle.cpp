#include "Particle.hpp"

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

    // Resize.
    scale += zoom;

    // Fade out.
    color.alpha -= (fade / 255.0);

    time_to_live--;

    // Die if out of time, invisible or shrunk to nothing.
    if((time_to_live <= 0) ||
            (color.alpha <= 0) ||
            (scale <= 0))
    {
        time_to_live = 0;
    }
}

Particle Particle::withDelta(const float delta)
{
    Particle p = *this;
    p.angular_velocity *= delta;
    p.fade *= delta;
    p.friction *= delta;
    p.time_to_live /= delta;
    p.velocity_x *= delta;
    p.velocity_y *= delta;
    p.zoom *= delta;
    return p;
}

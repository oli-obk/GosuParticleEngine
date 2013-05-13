#include <Gosu/Color.hpp>

//static_assert(sizeof(Gosu::Color)==4, "Gosu::Color doesn't have 4 bytes");

// Colour based on float values (0.0..1.0)
struct Color_f
{
    float red, green, blue, alpha;
    // default is white fully visible
    Color_f():red(1),green(1),blue(1),alpha(1) {}
    Color_f(Gosu::Color c)
    {
        red = float(c.red())/255.0;
        green = float(c.green())/255.0;
        blue = float(c.blue())/255.0;
        alpha = float(c.alpha())/255.0;
    }
    operator Gosu::Color() const
    {
        return Gosu::Color(alpha * 255,
                            red   * 255,
                            green * 255,
                            blue  * 255
            );
    }
};

// A single particle.
struct Particle
{
    // State.
    float x, y;
    // 0.0 draws left/upper corner of image at x,y
    // 1.0 draws right/lower corner of image at x,y
    // default is 0.5
    float center_x, center_y;
    // change of position per frame
    float velocity_x, velocity_y;
    // change of angle per frame
    uint16_t angular_velocity;

    Color_f color;

    // image alpha decreases by fade per frame
    float fade;
    // size of image at start
    float scale;
    // scale increases by zoom per frame
    float zoom;
    // percentage by which velocity is decreased per frame
    float friction;
    // angle the image is drawn at in fast_math indice steps
    uint16_t angle;

    // Time to die.
    uint16_t time_to_live;

    Particle Angle(float gosu_degrees) const;
    Particle AngularVelocity(float gosu_degrees_per_frame) const;
    Particle TimeToLive(uint16_t frames) const;

    Particle()
    {
        init(0, 0);
    }

    Particle(float _x, float _y)
    {
        init(_x, _y);
    }

    void init(float _x, float _y)
    {
        x = _x;
        y = _y;
        center_x = center_y = 0.5;
        velocity_x = velocity_y = 0;
        angular_velocity = 0;
        color.alpha = 1.0;
        color.blue = color.red = color.green = 1.0;
        fade = 0.0;
        scale = 1.0;
        zoom = 0.0;
        friction = 0.0;
        angle = 0;
        time_to_live = 0.0;
    }

    void update();
};

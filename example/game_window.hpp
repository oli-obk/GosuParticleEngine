#ifndef GAMEWINDOW_HPP
#define GAMEWINDOW_HPP

#include <Gosu/Font.hpp>
#include <Gosu/Window.hpp> // Base class: Gosu::Window
#include <Gosu/Bitmap.hpp>
#include <Gosu/Image.hpp>
#include <map>
#include "ParticleEmitter.hpp"

enum RenderLayer
{
    Particles,
    GUI
};

class GameWindow : public Gosu::Window
{
private:
    GameWindow(const GameWindow& rhs);
    GameWindow& operator=(const GameWindow& rhs);
    double update_time;
    Gosu::Font font;
    ParticleEmitter particle_emitter;
public:
    GameWindow();
    virtual ~GameWindow();
    virtual void draw();
    virtual void update();
    virtual void buttonUp(Gosu::Button btn);
};

#endif // GAMEWINDOW_HPP

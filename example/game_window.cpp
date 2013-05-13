#include <stdexcept>
#include <Gosu/Utility.hpp>
#include <Gosu/Math.hpp>
#include <Gosu/Image.hpp>
#include <Gosu/Text.hpp>
#include <iostream>
#include <sstream>
#include <Gosu/Graphics.hpp>
#include <Gosu/ImageData.hpp>
#include <Gosu/Inspection.hpp>
#include <Gosu/Timing.hpp>
#include "game_window.hpp"

GameWindow::GameWindow()
:Gosu::Window(1200, 800, false)
,font(graphics(), Gosu::defaultFontName(), 20)
,particle_emitter(graphics(), L"particle_arrow.png", RenderLayer::Particles, 150000)
{
}

GameWindow::~GameWindow()
{
}

void GameWindow::draw()
{
    particle_emitter.draw();

	std::wstringstream wss;
	wss << Gosu::fps();
	wss << L"fps - ";
    wss << update_time;
	wss << L"ms / ";
    wss << 1000/60;
    wss << L"ms - ";
    wss << particle_emitter.getCount();
    wss << L" particles";
	font.draw(wss.str(), 0, 0, RenderLayer::GUI);
	graphics().drawTriangle(input().mouseX(), input().mouseY(), Gosu::Color::GRAY,
							input().mouseX()+10, input().mouseY(), Gosu::Color::GRAY,
							input().mouseX(), input().mouseY()+10, Gosu::Color::GRAY, RenderLayer::GUI);

}

void GameWindow::update()
{
    auto start_time = Gosu::milliseconds();
    if (input().down(Gosu::msRight)) {
        for(int i = 0; i < 1000; i++) {
            Particle p(input().mouseX(), input().mouseY());
            p.scale = 0.1;
            p.color = Gosu::Color::AQUA;
            p.velocity_x = Gosu::random(-1, 1)/6;
            p.velocity_y = Gosu::random(-1, 1)/6;
            p.fade = 0.3;
            particle_emitter.emit(p.TimeToLive(1000));
        }
    }
    particle_emitter.update();
    update_time = Gosu::milliseconds() - start_time;
}

void GameWindow::buttonUp(Gosu::Button btn)
{
    if (btn == Gosu::msLeft) {
        Particle p(input().mouseX(), input().mouseY());
        p.fade = 10;
        particle_emitter.emit(p.TimeToLive(300).AngularVelocity(10));
    }
}

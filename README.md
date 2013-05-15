GosuParticleEngine
==================

A highly optimized Particle Engine to be used with Gosu ( http://libgosu.org ).
Ported from Spooner's Ashton library ( https://github.com/Spooner/ashton ).

Usage
==================

    ParticleEmitter emitter(Gosu::Graphics object from your window,
                        name of the image to be used with this emitter,
                        Z-Position of this emitter,
                        maximum number of particles);
                        
    emitter.update(); // run this every update call of your window
    emitter.draw(); // run this every draw call of your window

Particle Creation
==================

    Particle p(x, y);
    p.TimeToLive(1000); // lives for 1000 update calls (most likely that is around 16 seconds)
    p.Angle(90); // in gosu degrees, 90 is to the right, 0 is up, -90 is to the left
    p.AngularVelocity(2); // in gosu degrees, per update change, note: rounded to one decimal precision
    p.color = Gosu::Color::RED; // starting color
    p.fade = 0.3; // color alpha is decreased by this amount per update, if color alpha hits zero, the particle is erased
    p.velocity_x = 0.5; // moves half a pixel to the right every update
    p.scale = 10.0; // particle is rendered 10x as big as the actual image is
    p.zoom = -0.01; // scale is changed by this amount at every update
    p.friction = -0.1; // percentage velocity change every update. negative values will speedup the particle.
    emitter.emit(p);

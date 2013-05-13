#ifndef PARTICLE_EMITTER_HPP
#define PARTICLE_EMITTER_HPP

// this file was created by porting Spooner's ashton's particle emitter code to c++

#include <string>
#include <Gosu/Fwd.hpp>
#include <Gosu/Color.hpp>
#include <array>
#include <cstdint>
#include <Gosu/Image.hpp>
#include <Gosu/ImageData.hpp>
#include "Particle.hpp"

#define VERTICES_IN_PARTICLE 4

typedef struct _vertex2d
{
    float x, y;
} Vertex2d;


typedef std::vector<Particle> ParticleArray;
typedef ParticleArray::iterator ParticleIterator;
typedef std::vector<Vertex2d> VertexArray;
typedef VertexArray::iterator VertexIterator;
typedef std::vector<Gosu::Color> ColorArray;
typedef ColorArray::iterator ColorIterator;

class ParticleEmitter
{
    Gosu::Graphics& graphics;
    const Gosu::Image image;
    Gosu::ZPos z;
    size_t width; // Width of image.
    size_t height; // Height of image.
    Gosu::GLTexInfo texture_info; // Texture coords and id.

    ParticleArray particles;

    ColorArray color_array; // Color array.
    size_t color_array_offset; // Offset to colours within VBO.

    VertexArray texture_coords_array; // Tex coord array.
    size_t texture_coords_array_offset; // Offset to texture coords within VBO.

    VertexArray vertex_array; // Vertex array.
    size_t vertex_array_offset; // Offset to vertices within VBO.

    // VBO and client-side data arrays.
    unsigned int vbo_id;

    size_t count; // Current number of active particles.
    size_t max_particles; // No more will be created if max hit.
    ParticleIterator next_particle; // Next place to create a new particle (either dead or oldest living).

    // do not copy
    ParticleEmitter(const ParticleEmitter&) = delete;
    ParticleEmitter& operator=(const ParticleEmitter&) = delete;
    void init_vbo();
    void draw_vbo();
    void update_vbo();
    bool texture_changes() const;
    static bool initialized_fast_math;
    void write_texture_coords_for_all_particles();
    void write_texture_coords_for_particles(VertexIterator& texture_coord,
                                               ParticleIterator first, ParticleIterator end);
    void write_vertices_for_particles(VertexIterator& vertex,
                                         ParticleIterator first, ParticleIterator end);
    const float delta;
public:
    size_t getCount() const { return count; }
    ParticleEmitter(Gosu::Graphics& graphics, std::wstring filename, Gosu::ZPos z, size_t max_particles, const float delta);
    ~ParticleEmitter();
    void emit(Particle p);
    void update();
    void draw();
};

#endif // PARTICLE_EMITTER_HPP

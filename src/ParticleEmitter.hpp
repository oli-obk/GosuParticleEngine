#ifndef PARTICLE_EMITTER_HPP
#define PARTICLE_EMITTER_HPP

// this file was created by porting Spooner's ashton's particle emitter code to c++

#include <map>
#include <string>
#include <Gosu/Fwd.hpp>
#include <Gosu/Color.hpp>
#include <array>
#include <cstdint>
#include <Gosu/Image.hpp>
#include <Gosu/Math.hpp>
#include <ImageData.hpp>

#include <cstdint>

static_assert(sizeof(Gosu::Color)==4, "Gosu::Color doesn't have 4 bytes");

// Colour based on float values (0.0..1.0)
typedef struct _color_f
{
    float red, green, blue, alpha;
} Color_f;


#define VERTICES_IN_PARTICLE 4

// A single particle.
typedef struct _particle
{
    // State.
    float x, y;
    float center_x, center_y;
    float velocity_x, velocity_y;
    float angular_velocity;

    Color_f color;

    // Rate of change
    float fade;
    float scale;
    float zoom;
    float friction;
    float angle;

    // Time to die.
    float time_to_live;
} Particle;
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
    void update_particle(Particle& p, const float delta);
    void write_texture_coords_for_all_particles();
    void write_texture_coords_for_particles(VertexIterator& texture_coord,
                                               ParticleIterator first, ParticleIterator end);
    void write_vertices_for_particles(VertexIterator& vertex,
                                         ParticleIterator first, ParticleIterator end);
public:
    size_t getCount() const { return count; }
    ParticleEmitter(Gosu::Graphics& graphics, std::wstring filename, Gosu::ZPos z, size_t max_particles);
    ~ParticleEmitter();
    void emit(Particle p);
    void update(const float delta);
    void draw();
};

#endif // PARTICLE_EMITTER_HPP

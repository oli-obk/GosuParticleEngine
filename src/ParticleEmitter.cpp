#include "ParticleEmitter.hpp"

#define GL_GLEXT_PROTOTYPES
#include <GL/gl.h>
#include <cstring>
#include <Gosu/Graphics.hpp>
#include "fast_math.hpp"

static void write_particle_vertices(VertexIterator& vertex,
                                         Particle& particle,
                                         const uint width, const uint height);

static void write_particle_texture_coords(VertexIterator& texture_coord,
                                               Gosu::GLTexInfo texture_info);

static void write_particle_colors(ColorIterator& color_out, Color_f* color_in);
static void write_colors_for_particles(ColorIterator& color,
                                       ParticleIterator first, ParticleIterator end);


bool ParticleEmitter::initialized_fast_math = false;

ParticleEmitter::ParticleEmitter(Gosu::Graphics& graphics, std::wstring filename, Gosu::ZPos z, size_t max_particles, const float delta)
:graphics(graphics)
,image(graphics, filename)
,z(z)
,max_particles(max_particles)
,delta(delta)
{
    if (!initialized_fast_math) {
        initialize_fast_math();
        initialized_fast_math = true;
    }

    init_vbo();

    // default Particle constructor is just fine
    particles.resize(max_particles);
    next_particle = particles.begin();
    count = 0;

    // Pixel size of image.
    width = image.width();
    height = image.height();

    // Fill the array with all the same coords (won't be used if the image changes dynamically).
    texture_info = *image.getData().glTexInfo();

    write_texture_coords_for_all_particles();

    // Push whole array to graphics card.
    glBindBufferARB(GL_ARRAY_BUFFER_ARB, vbo_id);

    glBufferSubDataARB(GL_ARRAY_BUFFER_ARB, texture_coords_array_offset,
                       sizeof(Vertex2d) * VERTICES_IN_PARTICLE * max_particles,
                       texture_coords_array.data());

    glBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);
}

void ParticleEmitter::draw()
{
    if(count == 0) return;

    // Run the actual drawing operation at the correct Z-order.
    graphics.beginGL();
    draw_vbo();
    graphics.endGL();
}

void ParticleEmitter::draw_vbo()
{
    glEnable(GL_BLEND);
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, texture_info.texName);

    glBindBufferARB(GL_ARRAY_BUFFER_ARB, vbo_id);

    // Only use colour array if colours are dynamic. Otherwise a single colour setting is enough.
        glEnableClientState(GL_COLOR_ARRAY);
        glColorPointer(4, GL_UNSIGNED_BYTE, 0, (void*)color_array_offset);

    // Always use the texture array, even if it is static.
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    glTexCoordPointer(2, GL_FLOAT, 0, (void*)texture_coords_array_offset);

    // Vertex array will always be dynamic.
    glEnableClientState(GL_VERTEX_ARRAY);
    glVertexPointer(2, GL_FLOAT, 0, (void*)vertex_array_offset);

    glDrawArrays(GL_QUADS, 0, count * VERTICES_IN_PARTICLE);

    glDisableClientState(GL_COLOR_ARRAY);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    glDisableClientState(GL_VERTEX_ARRAY);

    glBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);
}

void ParticleEmitter::init_vbo()
{
    if(!GL_ARB_vertex_buffer_object)
    {
       throw std::runtime_error("Ashton::ParticleEmitter requires GL_ARB_vertex_buffer_object, which is not supported by your OpenGL");
    }

    int num_vertices = max_particles * VERTICES_IN_PARTICLE;

    color_array.resize(num_vertices);
    color_array_offset = 0;

    texture_coords_array.resize(num_vertices);
    texture_coords_array_offset = sizeof(Gosu::Color) * num_vertices;

    vertex_array.resize(num_vertices);
    vertex_array_offset = (sizeof(Gosu::Color) + sizeof(Vertex2d)) * num_vertices;

    // Create the VBO, but don't upload any data yet.
    int data_size = (sizeof(Gosu::Color) + sizeof(Vertex2d) + sizeof(Vertex2d)) * num_vertices;
    glGenBuffersARB(1, &vbo_id);
    glBindBufferARB(GL_ARRAY_BUFFER_ARB, vbo_id);
    glBufferDataARB(GL_ARRAY_BUFFER_ARB, data_size, NULL, GL_STREAM_DRAW_ARB);

    // Check the buffer was actually created.
    int buffer_size = 0;
    glGetBufferParameterivARB(GL_ARRAY_BUFFER_ARB, GL_BUFFER_SIZE_ARB, &buffer_size);
    if(buffer_size != data_size)
    {
        throw std::runtime_error("Failed to create a VBO to hold emitter data.");
    }

    glBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);
}

bool ParticleEmitter::texture_changes() const
{
    return false;
}

void ParticleEmitter::update()
{
    if(delta < 0.0) {
        throw std::runtime_error("delta must be >= 0");
    }

    if(count > 0)
    {
        for(Particle& particle:particles)
        {
            // Ignore particles that are already dead.
            if(particle.time_to_live > 0)
            {
                particle.update();
                if(particle.time_to_live <= 0) {
                    count -= 1;
                }
            }
        }
    }

    // Copy all the current data onto the graphics card.
    if(count > 0) update_vbo();
}

void ParticleEmitter::update_vbo()
{
    // Ensure that drawing order is correct by drawing in order of creation...

    // First, we draw all those from after the current, going up to the last one.
    auto first = next_particle;
    auto end = particles.end();
    ColorIterator color = color_array.begin();
    VertexIterator texCoord = texture_coords_array.begin();
    VertexIterator vertex = vertex_array.begin();
    write_colors_for_particles(color,
                                   first, end);
    if(texture_changes())
    {
        write_texture_coords_for_particles(texCoord,
                                           first, end);
    }
    write_vertices_for_particles(vertex, first, end);

    // When we copy the second half of the particles, we want to start writing further on.
    // therefore we keep the color, texCoord and vertex iterators

    // Then go from the first to the current.
    if(next_particle != particles.begin())
    {
        first = particles.begin();
        end = next_particle;
        write_colors_for_particles(color,
                                       first, end);

        if(texture_changes())
        {
            write_texture_coords_for_particles(texCoord,
                                               first, end);
        }

        write_vertices_for_particles(vertex,
                                     first, end);
    }

    // Upload the data, but only as much as we are actually using.
    glBindBufferARB(GL_ARRAY_BUFFER_ARB, vbo_id);
        glBufferSubDataARB(GL_ARRAY_BUFFER_ARB, color_array_offset,
                           sizeof(Gosu::Color) * VERTICES_IN_PARTICLE * count,
                           color_array.data());

    if(texture_changes())
    {
        glBufferSubDataARB(GL_ARRAY_BUFFER_ARB, texture_coords_array_offset,
                           sizeof(Vertex2d) * VERTICES_IN_PARTICLE * count,
                           texture_coords_array.data());
    }

    glBufferSubDataARB(GL_ARRAY_BUFFER_ARB, vertex_array_offset,
                       sizeof(Vertex2d) * VERTICES_IN_PARTICLE * count,
                       vertex_array.data());

    glBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);
}

ParticleEmitter::~ParticleEmitter()
{
    glDeleteBuffersARB(1, &vbo_id);
}

void ParticleEmitter::emit(Particle p)
{
    // Find the first dead particle in the heap, or overwrite the oldest one.
    Particle& particle = *next_particle;

    // If we are replacing an old one, remove it from the count and clear it to fresh.
    if(particle.time_to_live <= 0)
    {
        count++; // Dead or never been used.
    }

    // Lets move the index onto the next one, or loop around.
    next_particle++;
    if (next_particle == particles.end()) {
        next_particle = particles.begin();
    }

    particle = p.withDelta(delta);
}


// ----------------------------------------
static void write_particle_vertices(VertexIterator& vertex, Particle& particle,
                                         const uint width, const uint height)
{
    // Totally ripped this code from Gosu :$
    float sizeX = width * particle.scale;
    float sizeY = height * particle.scale;

    float offsX = fast_sin_deg(particle.angle);
    float offsY = fast_cos_deg(particle.angle);

    float distToLeftX   = +offsY * sizeX * particle.center_x;
    float distToLeftY   = -offsX * sizeX * particle.center_x;
    float distToRightX  = -offsY * sizeX * (1 - particle.center_x);
    float distToRightY  = +offsX * sizeX * (1 - particle.center_x);
    float distToTopX    = +offsX * sizeY * particle.center_y;
    float distToTopY    = +offsY * sizeY * particle.center_y;
    float distToBottomX = -offsX * sizeY * (1 - particle.center_y);
    float distToBottomY = -offsY * sizeY * (1 - particle.center_y);

    vertex->x = particle.x + distToLeftX  + distToTopX;
    vertex->y = particle.y + distToLeftY  + distToTopY;
    vertex++;

    vertex->x = particle.x + distToRightX + distToTopX;
    vertex->y = particle.y + distToRightY + distToTopY;
    vertex++;

    vertex->x = particle.x + distToRightX + distToBottomX;
    vertex->y = particle.y + distToRightY + distToBottomY;
    vertex++;

    vertex->x = particle.x + distToLeftX  + distToBottomX;
    vertex->y = particle.y + distToLeftY  + distToBottomY;
    vertex++;
}

// ----------------------------------------
// Calculate the vertices for all active particles
void ParticleEmitter::write_vertices_for_particles(VertexIterator& vertex,
                                         ParticleIterator first, ParticleIterator end)
{
    for(;first != end; first++)
    {
        Particle& particle = *first;
        if(particle.time_to_live > 0)
        {
            write_particle_vertices(vertex, particle, width, height);
        }
    }
}

// ----------------------------------------
static void write_particle_texture_coords(VertexIterator& texture_coord,
                                               Gosu::GLTexInfo texture_info)
{
    texture_coord->x = texture_info.left;
    texture_coord->y = texture_info.top;
    texture_coord++;

    texture_coord->x = texture_info.right;
    texture_coord->y = texture_info.top;
    texture_coord++;

    texture_coord->x = texture_info.right;
    texture_coord->y = texture_info.bottom;
    texture_coord++;

    texture_coord->x = texture_info.left;
    texture_coord->y = texture_info.bottom;
    texture_coord++;
}

// ----------------------------------------
// Write out texture coords, assuming image is animated.
void ParticleEmitter::write_texture_coords_for_particles(VertexIterator& texture_coord,
                                               ParticleIterator first, ParticleIterator end)
{
    for(;first != end; first++)
    {
        Particle& particle = *first;
        if(particle.time_to_live > 0)
        {
            write_particle_texture_coords(texture_coord, texture_info);
        }
    }
}

// ----------------------------------------
// Write all texture coords, assuming the image isn't animated.
void ParticleEmitter::write_texture_coords_for_all_particles()
{
    auto texture_coord = texture_coords_array.begin();
    for(uint i = 0; i < max_particles; i++)
    {
        write_particle_texture_coords(texture_coord, texture_info);
    }
}

// ----------------------------------------
static void write_particle_colors(ColorIterator& color_out, Color_f* color_in)
{
    // Convert the color from float to int (1/4 the data size).
    Gosu::Color color(  color_in->alpha*255,
                        color_in->red*255,
                        color_in->green*255,
                        color_in->blue*255);

    *color_out = color;
    color_out++;
    *color_out = color;
    color_out++;
    *color_out = color;
    color_out++;
    *color_out = color;
    color_out++;
}

// ----------------------------------------
static void write_colors_for_particles(ColorIterator& color,
                                       ParticleIterator first, ParticleIterator end)
{
    for(;first != end; first++)
    {
        Particle& particle = *first;
        if(particle.time_to_live > 0)
        {
            write_particle_colors(color, &particle.color);
        }
    }
}


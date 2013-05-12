#include "ParticleEmitter.hpp"

#define GL_GLEXT_PROTOTYPES
#include <GL/gl.h>
#include <cstring>
#include <Gosu/Graphics.hpp>
#include "fast_math.hpp"

static Vertex2d* write_particle_vertices(Vertex2d* vertex,
                                         Particle* particle,
                                         const uint width, const uint height);
static uint write_vertices_for_particles(Vertex2d* vertex,
                                         Particle* first, Particle* last,
                                         const uint width, const uint height);

static Vertex2d* write_particle_texture_coords(Vertex2d* texture_coord,
                                               TextureInfo* texture_info);
static void write_texture_coords_for_particles(Vertex2d* texture_coord,
                                               Particle* first, Particle* last,
                                               TextureInfo* texture_info);
static void write_texture_coords_for_all_particles(Vertex2d *texture_coord,
                                                   TextureInfo* texture_info,
                                                   const uint num_particles);

static Color_i* write_particle_colors(Color_i* color_out, Color_f* color_in);
static void write_colors_for_particles(Color_i* color,
                                       Particle* first, Particle* last);


bool ParticleEmitter::initialized_fast_math = false;

ParticleEmitter::ParticleEmitter(Gosu::Graphics& graphics, std::wstring filename, Gosu::ZPos z, size_t max_particles)
:graphics(graphics)
,image(graphics, filename)
,z(z)
,max_particles(max_particles)
{
    if (!initialized_fast_math) {
        initialize_fast_math();
        initialized_fast_math = true;
    }

    init_vbo();

    particles = new Particle[max_particles];
    memset(particles, 0, max_particles * sizeof(Particle));
    next_particle_index = 0;
    count = 0;

    // Pixel size of image.
    width = image.width();
    height = image.height();

    // Fill the array with all the same coords (won't be used if the image changes dynamically).
    const Gosu::GLTexInfo* tex_info = image.getData().glTexInfo();
    texture_info.id     = tex_info->texName;
    texture_info.left   = tex_info->left;
    texture_info.right  = tex_info->right;
    texture_info.top    = tex_info->top;
    texture_info.bottom = tex_info->bottom;

    write_texture_coords_for_all_particles(texture_coords_array,
                                           &texture_info,
                                           max_particles);

    // Push whole array to graphics card.
    glBindBufferARB(GL_ARRAY_BUFFER_ARB, vbo_id);

    glBufferSubDataARB(GL_ARRAY_BUFFER_ARB, texture_coords_array_offset,
                       sizeof(Vertex2d) * VERTICES_IN_PARTICLE * max_particles,
                       texture_coords_array);

    glBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);
}

bool ParticleEmitter::color_changes() const
{
    return true;
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
    glBindTexture(GL_TEXTURE_2D, texture_info.id);

    glBindBufferARB(GL_ARRAY_BUFFER_ARB, vbo_id);

    // Only use colour array if colours are dynamic. Otherwise a single colour setting is enough.
    if(color_changes())
    {
        glEnableClientState(GL_COLOR_ARRAY);
        glColorPointer(4, GL_UNSIGNED_BYTE, 0, (void*)color_array_offset);
    }
    else
    {
        //glColor4fv((GLfloat*)&color);
    }

    // Always use the texture array, even if it is static.
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    glTexCoordPointer(2, GL_FLOAT, 0, (void*)texture_coords_array_offset);

    // Vertex array will always be dynamic.
    glEnableClientState(GL_VERTEX_ARRAY);
    glVertexPointer(2, GL_FLOAT, 0, (void*)vertex_array_offset);

    glDrawArrays(GL_QUADS, 0, count * VERTICES_IN_PARTICLE);

    if(color_changes()) glDisableClientState(GL_COLOR_ARRAY);
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

    color_array = new Color_i[num_vertices];
    color_array_offset = 0;

    texture_coords_array = new Vertex2d[num_vertices];
    texture_coords_array_offset = sizeof(Color_i) * num_vertices;

    vertex_array = new Vertex2d[num_vertices];
    vertex_array_offset = (sizeof(Color_i) + sizeof(Vertex2d)) * num_vertices;

    // Create the VBO, but don't upload any data yet.
    int data_size = (sizeof(Color_i) + sizeof(Vertex2d) + sizeof(Vertex2d)) * num_vertices;
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

void ParticleEmitter::update(const float delta)
{
    if(delta < 0.0) {
        throw std::runtime_error("delta must be >= 0");
    }

    if(count > 0)
    {
        Particle* particle = particles;
        Particle* last = &particles[max_particles - 1];
        for(; particle <= last; particle++)
        {
            // Ignore particles that are already dead.
            if(particle->time_to_live > 0)
            {
                update_particle(particle, delta);
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
    Particle* first = &particles[next_particle_index];
    Particle* last = &particles[max_particles - 1];
    if(color_changes())
    {
        write_colors_for_particles(color_array,
                                   first, last);
    }
    if(texture_changes())
    {
        write_texture_coords_for_particles(texture_coords_array,
                                           first, last,
                                           &texture_info);
    }
    uint num_particles_written = write_vertices_for_particles(vertex_array,
                                                              first, last,
                                                              width, height);

    // When we copy the second half of the particles, we want to start writing further on.
    uint offset = num_particles_written * VERTICES_IN_PARTICLE;

    // Then go from the first to the current.
    if(next_particle_index > 0)
    {
        Particle* first = &particles[0];
        Particle* last = &particles[next_particle_index - 1];
        if(color_changes())
        {
            write_colors_for_particles(&color_array[offset],
                                       first, last);
        }

        if(texture_changes())
        {
            write_texture_coords_for_particles(&texture_coords_array[offset],
                                               first, last,
                                               &texture_info);
        }

        write_vertices_for_particles(&vertex_array[offset],
                                     first, last,
                                     width, height);
    }

    // Upload the data, but only as much as we are actually using.
    glBindBufferARB(GL_ARRAY_BUFFER_ARB, vbo_id);
    if(color_changes())
    {
        glBufferSubDataARB(GL_ARRAY_BUFFER_ARB, color_array_offset,
                           sizeof(Color_i) * VERTICES_IN_PARTICLE * count,
                           color_array);
    }

    if(texture_changes())
    {
        glBufferSubDataARB(GL_ARRAY_BUFFER_ARB, texture_coords_array_offset,
                           sizeof(Vertex2d) * VERTICES_IN_PARTICLE * count,
                           texture_coords_array);
    }

    glBufferSubDataARB(GL_ARRAY_BUFFER_ARB, vertex_array_offset,
                       sizeof(Vertex2d) * VERTICES_IN_PARTICLE * count,
                       vertex_array);

    glBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);
}

ParticleEmitter::~ParticleEmitter()
{
    glDeleteBuffersARB(1, &vbo_id);
}

void ParticleEmitter::update_particle(Particle* particle, const float delta)
{
    // Apply friction
    particle->velocity_x *= 1.0 - particle->friction * delta;
    particle->velocity_y *= 1.0 - particle->friction * delta;

    // Gravity.
    particle->velocity_y += /*gravity*/ 0.0 * delta;

    // Move
    particle->x += particle->velocity_x * delta;
    particle->y += particle->velocity_y * delta;

    // Rotate.
    particle->angle += particle->angular_velocity * delta;

    // Resize.
    particle->scale += particle->zoom * delta;

    // Fade out.
    particle->color.alpha -= (particle->fade / 255.0) * delta;

    particle->time_to_live -= delta;

    // Die if out of time, invisible or shrunk to nothing.
    if((particle->time_to_live <= 0) ||
            (particle->color.alpha <= 0) ||
            (particle->scale <= 0))
    {
        particle->time_to_live = 0;
        count -= 1;
    }
}

void ParticleEmitter::emit(Particle p)
{
    // Find the first dead particle in the heap, or overwrite the oldest one.
    Particle* particle = &particles[next_particle_index];

    // If we are replacing an old one, remove it from the count and clear it to fresh.
    if(particle->time_to_live > 0)
    {
        // Kill off and replace one with time to live :(
        memset(particle, 0, sizeof(Particle));
    }
    else
    {
        count++; // Dead or never been used.
    }

    // Lets move the index onto the next one, or loop around.
    next_particle_index = (next_particle_index + 1) % max_particles;

    *particle = p;
}


// ----------------------------------------
static Vertex2d* write_particle_vertices(Vertex2d* vertex, Particle* particle,
                                         const uint width, const uint height)
{
    // Totally ripped this code from Gosu :$
    float sizeX = width * particle->scale;
    float sizeY = height * particle->scale;

    float offsX = fast_sin_deg(particle->angle);
    float offsY = fast_cos_deg(particle->angle);

    float distToLeftX   = +offsY * sizeX * particle->center_x;
    float distToLeftY   = -offsX * sizeX * particle->center_x;
    float distToRightX  = -offsY * sizeX * (1 - particle->center_x);
    float distToRightY  = +offsX * sizeX * (1 - particle->center_x);
    float distToTopX    = +offsX * sizeY * particle->center_y;
    float distToTopY    = +offsY * sizeY * particle->center_y;
    float distToBottomX = -offsX * sizeY * (1 - particle->center_y);
    float distToBottomY = -offsY * sizeY * (1 - particle->center_y);

    vertex->x = particle->x + distToLeftX  + distToTopX;
    vertex->y = particle->y + distToLeftY  + distToTopY;
    vertex++;

    vertex->x = particle->x + distToRightX + distToTopX;
    vertex->y = particle->y + distToRightY + distToTopY;
    vertex++;

    vertex->x = particle->x + distToRightX + distToBottomX;
    vertex->y = particle->y + distToRightY + distToBottomY;
    vertex++;

    vertex->x = particle->x + distToLeftX  + distToBottomX;
    vertex->y = particle->y + distToLeftY  + distToBottomY;
    vertex++;

    return vertex;
}

// ----------------------------------------
// Calculate the vertices for all active particles
static uint write_vertices_for_particles(Vertex2d *vertex,
                                         Particle* first, Particle* last,
                                         const uint width, const uint height)
{
    int num_particles_written = 0;

    for(Particle* particle = first; particle <= last; particle++)
    {
        if(particle->time_to_live > 0)
        {
            vertex = write_particle_vertices(vertex, particle, width, height);
            num_particles_written++;
        }
    }

    return num_particles_written;
}

// ----------------------------------------
static Vertex2d* write_particle_texture_coords(Vertex2d* texture_coord,
                                               TextureInfo* texture_info)
{
    texture_coord->x = texture_info->left;
    texture_coord->y = texture_info->top;
    texture_coord++;

    texture_coord->x = texture_info->right;
    texture_coord->y = texture_info->top;
    texture_coord++;

    texture_coord->x = texture_info->right;
    texture_coord->y = texture_info->bottom;
    texture_coord++;

    texture_coord->x = texture_info->left;
    texture_coord->y = texture_info->bottom;
    texture_coord++;

    return texture_coord;
}

// ----------------------------------------
// Write out texture coords, assuming image is animated.
static void write_texture_coords_for_particles(Vertex2d *texture_coord,
                                               Particle* first, Particle* last,
                                               TextureInfo * texture_info)
{
    for(Particle* particle = first; particle <= last; particle++)
    {
        if(particle->time_to_live > 0)
        {
            texture_coord = write_particle_texture_coords(texture_coord, texture_info);
        }
    }
}

// ----------------------------------------
// Write all texture coords, assuming the image isn't animated.
static void write_texture_coords_for_all_particles(Vertex2d *texture_coord,
                                                   TextureInfo * texture_info,
                                                   const uint num_particles)
{
    for(uint i = 0; i < num_particles; i++)
    {
        texture_coord = write_particle_texture_coords(texture_coord, texture_info);
    }
}

// ----------------------------------------
static Color_i* write_particle_colors(Color_i* color_out, Color_f* color_in)
{
    // Convert the color from float to int (1/4 the data size).
    Color_i color;
    color.red = color_in->red * 255;
    color.green = color_in->green * 255;
    color.blue = color_in->blue * 255;
    color.alpha = color_in->alpha * 255;

    *color_out = color;
    color_out++;
    *color_out = color;
    color_out++;
    *color_out = color;
    color_out++;
    *color_out = color;
    color_out++;

    return color_out;
}

// ----------------------------------------
static void write_colors_for_particles(Color_i *color,
                                       Particle* first, Particle* last)
{
    for(Particle* particle = first; particle <= last; particle++)
    {
        if(particle->time_to_live > 0)
        {
            color = write_particle_colors(color, &particle->color);
        }
    }
}


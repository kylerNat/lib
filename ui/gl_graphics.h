#ifndef GRAPHICS
#define GRAPHICS

#include <gl/gl.h>
#include <utils/logging.h>
#include <utils/memory.h>
#include <utils/misc.h>

#include "graphics_common.h"

struct program_attribute
{
    GLint dim; //dimension
    GLenum type;
};

struct program_info
{
    GLuint program;

    program_attribute* attribs;
    int n_attribs;

    GLuint* uniforms;
    int n_uniforms;
};

struct shader_source
{
    GLenum type;
    char* filename;
    char* source;
};

struct vi_attribute
{
    GLuint index;
    GLint size;
    GLenum type;
    GLboolean normalized;
    GLsizei stride;
    GLvoid* pointer;
};

struct vi_buffer
{
    int n_attribs;
    vi_attribute* attribs;

    GLuint vertex_buffer;
    GLuint index_buffer;
    GLuint n_vertex_buffer;
    GLuint n_index_buffer;
};

vi_buffer vi_circle;

#define DEBUG_SEVERITY_HIGH                           0x9146
#define DEBUG_SEVERITY_MEDIUM                         0x9147
#define DEBUG_SEVERITY_LOW                            0x9148

void APIENTRY gl_error_callback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const char* message, const void* userParam)
{
    #ifndef GL_SUPPRESS_WARNINGS
    switch(severity)
    {
        case DEBUG_SEVERITY_HIGH:
            assert(0, "HIGH: ", message);
            break;
        case DEBUG_SEVERITY_MEDIUM:
            log_warning("MEDIUM: ");
            break;
        case DEBUG_SEVERITY_LOW:
            log_warning("LOW: ");
            break;
        default:
            log_output("NOTICE: ");
            break;
    }
    log_output(message, "\n\n");
    #endif //GL_SUPPRESS_WARNINGS
}

vi_buffer create_vertex_and_index_buffer(uint vb_size, void * vb_data,
                                         uint ib_size, void * ib_data,
                                         uint n_attribs, vi_attribute* attribs)
{
    vi_buffer out = {};
    glGenBuffers(1, &out.vertex_buffer);//TODO: maybe do this in bulk?
    glBindBuffer(GL_ARRAY_BUFFER, out.vertex_buffer);
    glBufferData(GL_ARRAY_BUFFER, vb_size*4, vb_data, GL_STATIC_DRAW);//TODO: use glNamedBufferData if available

    glGenBuffers(1, &out.index_buffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, out.index_buffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, ib_size*4, ib_data, GL_STATIC_DRAW);

    out.n_vertex_buffer = vb_size;
    out.n_index_buffer = ib_size;
    out.attribs = attribs;
    out.n_attribs = n_attribs;

    return out;
}

inline void bind_vertex_and_index_buffers(vi_buffer vi)
{
    glBindBuffer(GL_ARRAY_BUFFER, vi.vertex_buffer);
    // assert(pinfo.n_attribs >= vi.n_attribs, "number of attributes does not match");
    for(int i = 0; i < vi.n_attribs; i++)
    {
        //TODO: actually search through and match indices
        // assert(pinfo.attribs[i].index == vi.attribs[i].index);
        // assert(pinfo.attribs[i].size == vi.attribs[i].size);
        // assert(pinfo.attribs[i].type == vi.attribs[i].type);

        glEnableVertexAttribArray(i);
        glVertexAttribPointer(i,
                              vi.attribs[i].size,
                              vi.attribs[i].type,
                              vi.attribs[i].normalized,
                              vi.attribs[i].stride,
                              vi.attribs[i].pointer);
    }

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vi.index_buffer);
}

shader_source load_shader_from_file(GLenum type, char* filename)
{
    return {type, filename, load_file_0_terminated(filename)};
}

GLuint init_shader(shader_source source)
{
    GLuint shader = glCreateShader(source.type);
    assert(shader, "could not create shader for ", source.filename);
    glShaderSource(shader, 1, &source.source, 0);
    glCompileShader(shader);

    log_output("compiling shader:\n", source.source, "\n\n");
    int error;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &error);
    if(error == GL_FALSE)
    {
        int info_log_len = -1;
        int mem_id = reserve_stack();
        char* info_log = (char*) memory_stack_memory[mem_id];
        glGetShaderInfoLog(shader, available_free_memory(mem_id), &info_log_len, info_log);
        log_output("info log is ", info_log_len, " characters long\n");
        assert(0, "could not compile shader ", source.filename, ":\n", info_log);
        unreserve_stack(mem_id);
    }

    return shader;
}

GLuint init_program(size_t n_shaders, shader_source* sources)
{
    GLuint program = glCreateProgram();
    assert(program, "could not create program, GL error ", glGetError());

    int mem_id = reserve_stack();
    GLuint* shaders = (GLuint*) memory_stack_memory[mem_id];
    for(int i = 0; i < n_shaders; i++)
    {
        shaders[i] = init_shader(sources[i]);
        glAttachShader(program, shaders[i]);
    }

    glLinkProgram(program);

    int error;
    glGetProgramiv(program, GL_LINK_STATUS, &error);
    if(error == 0)
    {
        int mem_id = reserve_stack();
        char* info_log = (char*) memory_stack_memory[mem_id];
        glGetProgramInfoLog(program, available_free_memory(mem_id), 0, info_log);
        assert(0, info_log);
        unreserve_stack(mem_id);
    }

    for(int i = 0; i < n_shaders; i++)
    {
        glDetachShader(program, shaders[i]);
        glDeleteShader(shaders[i]);
    }
    unreserve_stack(mem_id);
    return program;
}

#define DEFAULT_HEADER "#version 440\n"
#define SHADER_SOURCE(source) #source
#define LONGEST_UNIFORM_NAME 32

#define define_program(program_name, attributes_list, uniform_list, shader_list) \
    program_info init_##program_name##_program()                        \
    {                                                                   \
        program_info pinfo;                                             \
                                                                        \
        const program_attribute attributes[] = {NOPAREN attributes_list}; \
                                                                        \
        pinfo.attribs = (program_attribute*) permalloc(sizeof(attributes)); \
        memcpy(pinfo.attribs, attributes, sizeof(attributes));          \
        pinfo.n_attribs = len(attributes);                              \
                                                                        \
        shader_source sources[] = {NOPAREN shader_list};                \
                                                                        \
        pinfo.program = init_program(len(sources), sources);            \
                                                                        \
        const char uniform_names[][LONGEST_UNIFORM_NAME] = {NOPAREN uniform_list}; \
        pinfo.n_uniforms = len(uniform_names);                          \
        pinfo.uniforms = (GLuint*) permalloc(len(uniform_names)*sizeof(pinfo.uniforms[0])); \
        for(int u = 0; u < len(uniform_list); u++)                      \
        {                                                               \
            pinfo.uniforms[u] =                                         \
                glGetUniformLocation(pinfo.program, uniform_names[u]);  \
        }                                                               \
        return pinfo;                                                   \
    };

define_program(
    flat_2d,
    ( //vertex attributes
    {3, GL_FLOAT},
    {3, GL_FLOAT},
    {2, GL_FLOAT},
    {3, GL_FLOAT}
        ),
    ( //uniforms
        "t"),
    ( //shaders
    {GL_VERTEX_SHADER, "<default 2d flat vertex shader>",
            DEFAULT_HEADER SHADER_SOURCE(
                /////////////////<default 2d flat vertex shader>/////////////////
                layout(location = 0) in vec3 r;
                layout(location = 1) in vec3 R;
                layout(location = 2) in vec2 dir;
                layout(location = 3) in vec4 vert_c;

                smooth out vec4 c;

                uniform mat3 t;

                void main()
                {
                    vec3 pos = vec3(0);
                    pos.xy = r.yx*dir.y;
                    pos.x *= -1;
                    pos.xy += r.xy*dir.x;
                    pos.z = r.z;
                    gl_Position.xyz = t*(pos+R.xyz);
                    c = vert_c;
                }
                /*/////////////////////////////////////////////////////////////*/)},
    {GL_FRAGMENT_SHADER, "<default 2d flat fragment shader>",
            DEFAULT_HEADER SHADER_SOURCE(
                ////////////////<default 2d flat fragment shader>////////////////
                layout(location = 0) out vec4 frag_color;

                smooth in vec4 c;

                void main()
                {
                    frag_color = c;
                }
                /*/////////////////////////////////////////////////////////////*/)}
        ));

define_program(
    textured_2d,
    ( //vertex attributes
    {3, GL_FLOAT},
    {3, GL_FLOAT},
    {2, GL_FLOAT},
    {2, GL_FLOAT}
        ),
    ( //uniforms
        "t",
        "tex"),
    ( //shaders
    {GL_VERTEX_SHADER, "<default 2d textured vertex shader>",
            DEFAULT_HEADER SHADER_SOURCE(
                /////////////////<default 2d textured vertex shader>/////////////////
                layout(location = 0) in vec3 r;
                layout(location = 1) in vec3 R;
                layout(location = 2) in vec2 dir;
                layout(location = 3) in vec2 vert_uv;

                smooth out vec2 uv;

                uniform mat3 t;

                void main()
                {
                    vec3 pos = vec3(0);
                    pos.xy = r.yx*dir.y;
                    pos.x *= -1;
                    pos.xy += r.xy*dir.x;
                    pos.z = r.z;
                    gl_Position.xyz = t*(pos+R.xyz);
                    uv = 0.5*(r.xy+vec2(1,1));// vert_uv;
                }
                /*/////////////////////////////////////////////////////////////*/)},
    {GL_FRAGMENT_SHADER, "<default 2d textured fragment shader>",
            DEFAULT_HEADER SHADER_SOURCE(
                ////////////////<default 2d textured fragment shader>////////////////
                layout(location = 0) out vec4 frag_color;

                smooth in vec2 uv;

                uniform sampler2D tex;

                void main()
                {
                    vec2 r = 2*(uv-vec2(0.5, 0.5));
                    frag_color.rgb = vec3(1,1,1);
                    frag_color.a = 1.0;
                }
                /*/////////////////////////////////////////////////////////////*/)}
        ));

define_program(
    circle,
    ( //vertex attributes
    {3, GL_FLOAT},
    {3, GL_FLOAT},
    {1, GL_FLOAT},
    {4, GL_FLOAT},
        ), //TODO: reduce repetition of this stuff
    ( //uniforms
        "t",
        "res"),
    ( //shaders
    {GL_VERTEX_SHADER, "<default 2d circle vertex shader>",
            DEFAULT_HEADER SHADER_SOURCE(
                /////////////////<default 2d circle vertex shader>/////////////////
                layout(location = 0) in vec3 x;
                layout(location = 1) in vec3 X;
                layout(location = 2) in float r;
                layout(location = 3) in vec4 c;

                smooth out vec4 color;
                smooth out vec2 uv;
                smooth out float r_px;

                uniform mat3 t;
                uniform float res;

                void main()
                {
                    gl_Position.xyz = t*(x*r+X);
                    uv = res*0.5*(x.xy*r);
                    r_px = res*0.5*r;
                    color = c;
                }
                /*/////////////////////////////////////////////////////////////*/)},
    {GL_FRAGMENT_SHADER, "<default 2d circle fragment shader>",
            DEFAULT_HEADER SHADER_SOURCE(
                ////////////////<default 2d circle fragment shader>////////////////
                layout(location = 0) out vec4 frag_color;

                smooth in vec4 color;
                smooth in vec2 uv;
                smooth in float r_px;

                // uniform sampler2D tex;

                void main()
                {
                    vec2 r = uv;
                    frag_color = color;
                    //TODO: the sharpness scaling should be resolution dependent
                    frag_color.a *= smoothstep(r_px, r_px-1, length(r));
                }
                /*/////////////////////////////////////////////////////////////*/)}
        ));

program_info pinfo_flat_2d;
program_info pinfo_textured_2d;
program_info pinfo_circle;

void load_default_programs()
{
    pinfo_flat_2d = init_flat_2d_program();
    pinfo_textured_2d = init_textured_2d_program();
    pinfo_circle = init_circle_program();
}

struct attribute_value
{
    GLuint index;
    int divisor;
    int stride;
    int offset_width;
};

inline uint gl_get_type_size(GLenum type)
{
    switch(type)
    {
        case GL_BYTE: return sizeof(GLbyte);
        case GL_UNSIGNED_BYTE: return sizeof(GLubyte);
        case GL_SHORT: return sizeof(GLshort);
        case GL_UNSIGNED_SHORT: return sizeof(GLushort);
        case GL_INT: return sizeof(GLint);
        case GL_UNSIGNED_INT: return sizeof(GLuint);
        case GL_HALF_FLOAT: return 16;
        case GL_FLOAT: return sizeof(GLfloat);
        case GL_DOUBLE: return sizeof(GLdouble);
        case GL_FIXED: return sizeof(GLfixed);
        case GL_INT_2_10_10_10_REV: return sizeof(GLint);
        case GL_UNSIGNED_INT_2_10_10_10_REV: return sizeof(GLuint);
        case GL_UNSIGNED_INT_10F_11F_11F_REV: return sizeof(GLuint);
        default: log_error("Unknown GL type: ", type);
    }
}

#define N_GENERAL_BUFFERS 2
#define GENERAL_BUFFER_SIZE 100*megabyte
GLuint gl_general_buffers[N_GENERAL_BUFFERS];

void gl_init_general_buffers()
{
    glGenBuffers(N_GENERAL_BUFFERS, gl_general_buffers);
    for(int i = 0; i < N_GENERAL_BUFFERS; i++)
    {
        glBindBuffer(GL_ARRAY_BUFFER, gl_general_buffers[i]);
        glBufferData(GL_ARRAY_BUFFER, GENERAL_BUFFER_SIZE, NULL, GL_STREAM_DRAW);
    }
}

#define add_attribute_common(dim, gl_type, do_normalize, stride, offset_advance, divisor) \
    glEnableVertexAttribArray(layout_location);                         \
    glVertexAttribPointer(layout_location, dim, gl_type, do_normalize, stride, (void*) (offset)); \
    offset += offset_advance;                                           \
    glVertexAttribDivisor(layout_location++, divisor);

#define add_contiguous_attribute(dim, gl_type, do_normalize, stride, offset_advance) \
    add_attribute_common(dim, gl_type, do_normalize, stride, offset_advance, 0);

#define add_interleaved_attribute(dim, gl_type, do_normalize, stride)   \
    add_attribute_common(dim, gl_type, do_normalize, stride, dim*gl_get_type_size(gl_type), 1);

//TODO:
int window_width = 1;
int window_height = 1;

void draw_circles(circle_render_info* circles, int n_circles)
{
    GLuint circle_buffer = gl_general_buffers[0];

    glUseProgram(pinfo_circle.program);

    //TODO: let this be controlled
    real transform[] = {
        1.0*window_height/window_width, 0, 0,
        0, 1.0, 0,
        0, 0, 1,
    };
    glUniformMatrix3fv(pinfo_circle.uniforms[0], 1, true, transform);
    glUniform1f(pinfo_circle.uniforms[1], window_height);

    glBindBuffer(GL_ARRAY_BUFFER, circle_buffer);

    size_t offset = 0;
    int layout_location = 0;

    //buffer square coord data (bounding box of circle)
    real vb[] = {-1,-1,0,
                 -1,+1,0,
                 +1,+1,0,
                 +1,-1,0};
    glBufferSubData(GL_ARRAY_BUFFER, offset, sizeof(vb), (void*) vb);

    add_contiguous_attribute(3, GL_FLOAT, false, 0, sizeof(vb)); //vertex positions

    //buffer circle data
    glBufferSubData(GL_ARRAY_BUFFER, offset, n_circles*sizeof(circles[0]), (void*) circles);
    add_interleaved_attribute(3, GL_FLOAT, false, sizeof(circles[0])); //center position
    add_interleaved_attribute(1, GL_FLOAT, false, sizeof(circles[0])); //radius
    add_interleaved_attribute(4, GL_FLOAT, false, sizeof(circles[0])); //RGBA color

    glDrawArraysInstanced(GL_TRIANGLE_FAN, 0, 4, n_circles);
}

#if 0
GLuint gl_new_cloud_texture()
{
    //create and allocate texture
    GLuint density_texture;
    glGenTextures(1, &density_texture);
    glBindTexture(GL_TEXTURE_3D, density_texture);
    glTexStorage3D(GL_TEXTURE_3D, 1, GL_R16_SNORM, Nx, Ny, Nz);

    //upload data, TODO: I will probably want to just integrate this with parallel computation stuff
    glTexSubImage3D(GL_TEXTURE_3D, 0, x0, y0, z0, Deltax, Deltay, Deltaz, GL_RED, GL_SHORT, density_data);
}

//draws a density cloud
void draw_cloud(real* cloud)
{
    GLuint cloud_buffer = gl_general_buffers[0];

    glUseProgram(pinfo_cloud.program);

    //TODO: let this be controlled
    real transform[] = {
        1, 0, 0,
        0, 1, 0,
        0, 0, 1,
    };
    glUniformMatrix3fv(pinfo_cloud.uniforms[0], 1, true, transform);

    glBindBuffer(GL_ARRAY_BUFFER, cloud_buffer);

    size_t offset = 0;
    int layout_location = 0;

    //buffer square coord data (bounding box of cloud)
    real vb[] = {-1,-1,0,
                 -1,+1,0,
                 +1,+1,0,
                 +1,-1,0};
    glBufferSubData(GL_ARRAY_BUFFER, offset, sizeof(vb), (void*) vb);

    add_contiguous_attribute(3, GL_FLOAT, false, 0, sizeof(vb));

    //buffer cloud data
    glBufferSubData(GL_ARRAY_BUFFER, offset, n_cloud*sizeof(circles[0]), (void*) circles);
    add_interleaved_attribute(3, GL_FLOAT, false, sizeof(cloud[0]));
    add_interleaved_attribute(1, GL_FLOAT, false, sizeof(cloud[0]));
    add_interleaved_attribute(4, GL_FLOAT, false, sizeof(cloud[0]));

    glDrawArraysInstanced(GL_TRIANGLE_FAN, 0, 4, n_circles);
}
#endif

#endif

#ifndef GRAPHICS
#define GRAPHICS

#include <gl/gl.h>
#include <utils/logging.h>
#include <utils/memory.h>
#include <utils/misc.h>

#include "graphics_common.h"

struct program_attribute
{
    GLint size; //dimension
    GLenum type;
};

struct program_info
{
    GLuint program;

    int n_attribs;
    program_attribute* attribs;

    int n_uniforms;
    GLuint* uniforms;
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
        char* info_log = (char*) free_memory;
        glGetShaderInfoLog(shader, free_memory_size, &info_log_len, info_log);
        log_output("info log is ", info_log_len, " characters long\n");
        assert(0, "could not compile shader ", source.filename, ":\n", info_log);
    }

    return shader;
}

GLuint init_program(size_t n_shaders, shader_source* sources)
{
    GLuint program = glCreateProgram();
    assert(program, "could not create program, GL error ", glGetError());

    GLuint* shaders = (GLuint*) free_memory; //TODO: this is not super safe since other functions might also use free_memory
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
        char* info_log = (char*) free_memory;
        glGetProgramInfoLog(program, free_memory_size, 0, info_log);
        assert(0, info_log);
    }

    for(int i = 0; i < n_shaders; i++)
    {
        glDetachShader(program, shaders[i]);
        glDeleteShader(shaders[i]);
    }
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
    circle_2d,
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
    {GL_VERTEX_SHADER, "<default 2d circle vertex shader>",
            DEFAULT_HEADER SHADER_SOURCE(
                /////////////////<default 2d circle vertex shader>/////////////////
                layout(location = 0) in vec3 r;
                layout(location = 1) in vec3 R;
                layout(location = 2) in float a;

                smooth out vec2 uv;
                smooth out float sharpness;

                uniform mat3 t;

                void main()
                {
                    gl_Position.xyz = t*(r*a+R);
                    uv = 0.5*(r.xy+vec2(1,1));// vert_uv;
                    sharpness = a;
                }
                /*/////////////////////////////////////////////////////////////*/)},
    {GL_FRAGMENT_SHADER, "<default 2d circle fragment shader>",
            DEFAULT_HEADER SHADER_SOURCE(
                ////////////////<default 2d circle fragment shader>////////////////
                layout(location = 0) out vec4 frag_color;

                smooth in vec2 uv;
                smooth in float sharpness;

                uniform sampler2D tex;

                void main()
                {
                    vec2 r = 2*(uv-vec2(0.5, 0.5));
                    frag_color.rgb = vec3(1,1,1);
                    //TODO: the sharpness scaling should be resolution dependent
                    frag_color.a = smoothstep(1.0, 1.0-1.0/(250*sharpness), length(r));
                }
                /*/////////////////////////////////////////////////////////////*/)}
        ));

program_info pinfo_flat_2d;
program_info pinfo_textured_2d;
program_info pinfo_circle_2d;

void load_default_programs()
{
    pinfo_flat_2d = init_flat_2d_program();
    pinfo_textured_2d = init_textured_2d_program();
    pinfo_circle_2d = init_circle_2d_program();
}

struct circle_drawing_buffer
{
    GLuint gl_buffer;
    GLuint index_buffer;
    real* data_buffer;
    size_t buffer_size;
};

circle_drawing_buffer make_circle_drawing_buffer(uint n_max_circles, uint n_circle_properties)
{
    GLuint point_buffer;
    GLuint index_buffer;
    size_t buffer_size = 4*3*sizeof(real) + n_max_circles*n_circle_properties*sizeof(real);

    real* circles = (real*) permalloc(buffer_size);

    glGenBuffers(1, &point_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, point_buffer);
    // glBufferData(GL_ARRAY_BUFFER, buffer_size, circles, GL_STREAM_DRAW);

    glBufferData(GL_ARRAY_BUFFER, n_max_circles*(3+2+3)*sizeof(circles[0]), NULL, GL_STREAM_DRAW);
    real vb[] = {-1,-1,+1,
                 -1,+1,+1,
                 +1,+1,+1,
                 +1,-1,+1};
    glBufferSubData(GL_ARRAY_BUFFER, 0 /*offset*/, 4*3*sizeof(real), (void*) vb);

    uint16 ib_data[] = {0,1,2,3};
    glGenBuffers(1, &index_buffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_buffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, 4*4, ib_data, GL_STATIC_DRAW);

    return {point_buffer, index_buffer, circles, buffer_size};
}


void end_draw_circles(uint n_max_circles, uint n_circles, GLuint circle_buffer, GLuint index_buffer, real* circles)
{
    // glEnd();

    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);
    // glEnableVertexAttribArray(3);
    glBindBuffer(GL_ARRAY_BUFFER, circle_buffer);
    glBufferSubData(GL_ARRAY_BUFFER, 12*4 /*offset*/, n_circles*(3+2*3)*(sizeof(circles[0])), (void*) circles);
    glVertexAttribPointer(0,
                          3,
                          GL_FLOAT,
                          false,
                          0,
                          (void*) 0);
    glVertexAttribPointer(1,
                          4,
                          GL_FLOAT,
                          false,
                          (3+2+3)*sizeof(circles[0]),
                          (void*) (12*4));
    glVertexAttribPointer(2,
                          4,
                          GL_FLOAT,
                          false,
                          (3+2+3)*sizeof(circles[0]),
                          (void*) (12*4+3*4));
    // glVertexAttribPointer(3,
    //                       4,
    //                       GL_FLOAT,
    //                       false,
    //                       (3+2+3)*sizeof(circles[0]),
    //                       (void*) ((3+2)*4));

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_buffer);

    glVertexAttribDivisor(0, 0);
    glVertexAttribDivisor(1, 1);
    glVertexAttribDivisor(2, 1);
    // glVertexAttribDivisor(3, 0);
    glDrawArraysInstanced(GL_TRIANGLE_FAN, 0, 4, n_circles);
}

struct attribute_value
{
    GLuint index;
    int divisor;
};

struct render_task
{
    program_info pinfo;
    attribute_value* attribs;
    int n_attribs;
    void** buffer_data;
    uint* n_buffer_elements;
    uint* buffer_element_size;
    uint n_buffers;
};

render_task* render_queue;
int n_render_queue;

uint gl_get_type_size(GLenum type)
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


#define N_MAX_BUFFERS 10
GLuint buffers[N_MAX_BUFFERS];

void init_render_buffers()
{
    glGenBuffers(N_MAX_BUFFERS, buffers);
    // glBufferData(GL_ARRAY_BUFFER, max_data_size, NULL, GL_STREAM_DRAW);
    // glBufferSubData(GL_ARRAY_BUFFER, 0 /*offset*/, data_size, task.data);
}

void flush_render_queue()
{
    for(int i = 0; i < n_render_queue; i++)
    {
        // glEnd();
        render_task task = render_queue[i];

        //put data in buffers
        for(int b = 0; b < task.n_buffers; b++)
        {
            uint data_size = task.buffer_element_size[b]*task.n_buffer_elements[b];
            glBindBuffer(GL_ARRAY_BUFFER, buffers[b]);
            glBufferData(GL_ARRAY_BUFFER, data_size, task.buffer_data[b], GL_STREAM_DRAW); //TODO: allow control over draw mode
        }

        int attrib_offset = 0;
        for(int a = 0; a < task.n_attribs; a++)
        {
            auto index = task.attribs[a].index;
            glVertexAttribDivisor(index, task.attribs[a].divisor);
            if(task.attribs[a].divisor == 1)
            {
                glEnableVertexAttribArray(index);
                glVertexAttribPointer(index,
                                      task.pinfo.attribs[index].size,
                                      task.pinfo.attribs[index].type,
                                      false, //normalize
                                      0, //stride between consecutive elements
                                      (void*) (attrib_offset));
                attrib_offset += task.pinfo.attribs[index].size*gl_get_type_size(task.pinfo.attribs[index].type);
            }
            else if(task.attribs[a].divisor == 0)
            {
                //TODO:
            }
            else
            {
                log_error("Unsupported value for glVertexAttribDivisor:", task.attribs[a].divisor, "\nSorry :(");
            }
        }
        glDrawArraysInstanced(GL_POLYGON, 0, vi_circle.n_index_buffer, task.n_buffer_elements[0]);
    }
}

#endif

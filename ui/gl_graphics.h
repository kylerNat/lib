#ifndef GRAPHICS
#define GRAPHICS

#include <gl/gl.h>
#include <utils/logging.h>
#include <utils/memory.h>

struct program_attribute
{
    GLuint index;
    GLint size;
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

program_info pinfo_2d_flat;
program_info pinfo_2d_texture;

program_info pinfo_3d_flat;
program_info pinfo_3d_depth;
program_info pinfo_3d_texture;

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

        glEnableVertexAttribArray(vi.attribs[i].index);
        glVertexAttribPointer(vi.attribs[i].index,
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

    log_output("compiling shader:\n", source.source, "\n");
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

void load_default_shaders()
{
    const program_attribute flat_2d_attributes[] = {
        {0, 3, GL_FLOAT},
        {0, 4, GL_FLOAT},
        {0, 4, GL_FLOAT},
    };
    pinfo_2d_flat.attribs = (program_attribute*) permalloc(sizeof(flat_2d_attributes));
    memcpy(pinfo_2d_flat.attribs, flat_2d_attributes, sizeof(flat_2d_attributes));
    pinfo_2d_flat.n_attribs = len(flat_2d_attributes);

    shader_source flat_2d_sources[] = {
        {GL_VERTEX_SHADER, "<default 2d flat vertex shader>",
         DEFAULT_HEADER SHADER_SOURCE(
             /////////////////<default 2d flat vertex shader>/////////////////
             layout(location = 0) in vec4 v;
             layout(location = 1) in vec4 r;
             layout(location = 2) in vec4 c;

             smooth out vec4 color;

             uniform mat3 t;

             void main()
             {
                 gl_Position.xyz = t*v.xyz*r.w+r.xyz;
                 color = c;
             }
             /*/////////////////////////////////////////////////////////////*/)
        },
        {GL_FRAGMENT_SHADER, "<default 2d flat fragment shader>",
         DEFAULT_HEADER SHADER_SOURCE(
             ////////////////<default 2d flat fragment shader>////////////////
             // uniform vec4 color;
             smooth in vec4 color;

             void main()
             {
                 gl_FragColor = color;
             }
             /*/////////////////////////////////////////////////////////////*/)
        },
    };

    pinfo_2d_flat.program = init_program(len(flat_2d_sources), flat_2d_sources);

    pinfo_2d_flat.uniforms = (GLuint*) free_memory;
    pinfo_2d_flat.n_uniforms = 0;
    pinfo_2d_flat.uniforms[pinfo_2d_flat.n_uniforms++] = glGetUniformLocation(pinfo_2d_flat.program, "t");
    // pinfo_2d_flat.uniforms[pinfo_2d_flat.n_uniforms++] = glGetUniformLocation(pinfo_2d_flat.program, "color");
    permalloc(pinfo_2d_flat.n_uniforms*sizeof(pinfo_2d_flat.uniforms[0]));

    //CIRCLE
    int n_lines = 10;

    vi_attribute attribs_list[] = {
        {
            0, //index
            3, //size
            GL_FLOAT, //type
            false, //normalized
            12, //stride
            0, //start
        },
        // {
        //     1, //index
        //     3, //size
        //     GL_FLOAT, //type
        //     false, //normalized
        //     24, //stride
        //     (GLvoid*) 12, //start
        // },
    };
    vi_attribute* attribs = (vi_attribute*) permalloc(sizeof(attribs_list));
    memcpy(attribs, attribs_list, sizeof(attribs_list));

    real* vb = ((real*) free_memory)+n_lines;
    int n_vb = 0;

    uint16* ib = ((uint16*) free_memory);
    int n_ib = 0;

    for(int i = 0; i < n_lines; i++)
    {
        vb[n_vb++] = cos(2*pi*i/n_lines);
        vb[n_vb++] = sin(2*pi*i/n_lines);
        vb[n_vb++] = 1;

        // vb[n_vb++] = 1;
        // vb[n_vb++] = 1;
        // vb[n_vb++] = 1;

        ib[n_ib++] = i;
    }

    vi_circle = create_vertex_and_index_buffer(n_vb, vb,
                                               n_ib, ib,
                                               len(attribs_list),  attribs);
}

enum render_tast_type
{
    rt_dots,
    rt_count,
};

struct render_task
{
    render_tast_type type;
};

render_task* render_queue;

#endif

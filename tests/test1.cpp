#include <ui/win32_gl_window.h>
#include <maths/maths.h>

struct real2
{
    real x;
    real y;
};

real2 operator+(real2 a, real2 b)
{
    return {a.x+b.x, a.y+b.y};
}

real2 operator+=(real2& a, real2 b)
{
    return a=a+b;
}

real2 operator*(real s, real2 v)
{
    return {s*v.x, s*v.y};
}

real2 operator*(real2 v, real s)
{
    return {s*v.x, s*v.y};
}

real2 operator*=(real2& v, real s)
{
    return v=v*s;
}

real dot(real2 a, real2 b)
{
    return a.x*b.x+a.y*b.y;
}

struct particle
{
    real2 r;
    real2 v;
};

int mymain()
{
    log_output("mymain\n");

    init_memory(); //TODO: figure out what the user actually needs to do
    window_t wnd = create_window("Test Window", "test window");

    load_default_shaders();

    log_output("created window\n");
    show_window(wnd);
    log_output("showing window\n");

    real t = 0;

    int N_MAX_PARTICLES = 1000000;
    particle* ps = (particle*) permalloc(N_MAX_PARTICLES*sizeof(particle));
    int n_ps = 5;
    int next_ps = n_ps;

    for(int i = 0; i < n_ps; i++)
    {
        real theta = i*2*pi/n_ps;
        real r = 1.0;
        real p = 1.0;
        ps[i] = {r*cos(theta), r*sin(theta), p*sin(theta), -p*cos(theta)};
    }

    // srand(time(NULL));

    GLuint circle_r_buffer;
    GLuint circle_c_buffer;
    {
        circles = (real*) permalloc(N_MAX_CIRCLES*2*4*sizeof(circles[0]));
        // circles = (real*) permalloc(N_MAX_CIRCLES*4*sizeof(circles[0]));

        glGenBuffers(1, &circle_r_buffer);
        glBindBuffer(GL_ARRAY_BUFFER, circle_r_buffer);
        glBufferData(GL_ARRAY_BUFFER, N_MAX_CIRCLES*4*sizeof(circles[0]), circles, GL_STREAM_DRAW);

        int gl_buffer_size;
        glGetBufferParameteriv(GL_ARRAY_BUFFER, GL_BUFFER_SIZE, &gl_buffer_size);
        log_output("circle_r_buffer size: ", gl_buffer_size, " (desired size = ", N_MAX_CIRCLES*4*sizeof(circles[0]), ")\n");

        glGenBuffers(1, &circle_c_buffer);
        glBindBuffer(GL_ARRAY_BUFFER, circle_c_buffer);
        glBufferData(GL_ARRAY_BUFFER, N_MAX_CIRCLES*4*sizeof(circles[0]), circles, GL_STREAM_DRAW);

        glGetBufferParameteriv(GL_ARRAY_BUFFER, GL_BUFFER_SIZE, &gl_buffer_size);
        log_output("circle_c_buffer size: ", gl_buffer_size, "\n");
    }

    bind_vertex_and_index_buffers(vi_circle);
    glVertexAttribDivisor(0, 0);
    glVertexAttribDivisor(1, 1);
    glVertexAttribDivisor(2, 1);
    real dt = 0.01;
    while(update_window(wnd))
    {
        {
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
            glUseProgram(pinfo_2d_flat.program);

            real transform[] = {
                1, 0, 0,
                0, 1, 0,
                0, 0, 1,
            };
            glUniformMatrix3fv(pinfo_2d_flat.uniforms[0], 1, true, transform);

            n_circles = 0;
            // glBegin(GL_TRIANGLES);
        }

        real energy = 0;
        t += dt;
        for(int i = 0; i < n_ps; i++)
        {
            real tau = i;// *2*pi/N_MAX_PARTICLES;
            real omega_color = 2*pi/5;

            real old_energy = dot(ps[i].v, ps[i].v);

            ps[i].r += dt*ps[i].v;
            ps[i].v += dt*(
                (1.0 - 0.5*sqrt(dot(ps[i].r, ps[i].r)))*(real2){ps[i].v.y, -ps[i].v.x}
                );

            energy = dot(ps[i].v, ps[i].v);

            ps[i].v *= sqrt(old_energy/energy);//0.9999;

            if(next_ps < N_MAX_PARTICLES)
            { //spawn new particles
                next_ps %= N_MAX_PARTICLES;
                int i_spawn = next_ps++;
                ps[i_spawn] = ps[i];
                float theta = (rand()%100)*(2.0f*pi/100.0f);
                float speed = 0.001*((rand()%100)*(0.1f/100.0f)+0.1);
                ps[i_spawn].v *= 1.0-0.1*(rand()%100)*(1/100.0f);
                ps[i_spawn].v += {speed*cos(theta), speed*sin(theta)};
            }

            draw_circle(wnd,
                        0.2*ps[i].r.x, 0.2*ps[i].r.y,
                        0.002,
                        cos(omega_color*tau), cos(omega_color*(tau+pi*2/3)), cos(omega_color*(tau+pi*4/3)));
        }

        if(next_ps > n_ps) n_ps = next_ps;

        // draw_circle(wnd, 1, 0, 0.05, 1, 1, 1);

        {
            // glEnd();

            glEnableVertexAttribArray(1);
            glEnableVertexAttribArray(2);
            glBindBuffer(GL_ARRAY_BUFFER, circle_r_buffer);
            glBufferData(GL_ARRAY_BUFFER, N_MAX_CIRCLES*2*4*sizeof(circles[0]), NULL, GL_STREAM_DRAW);
            glBufferSubData(GL_ARRAY_BUFFER, 0, n_circles*32, (void*) circles);
            glVertexAttribPointer(1,
                                  4,
                                  GL_FLOAT,
                                  false,
                                  32,
                                  (void*) 0);
            glVertexAttribPointer(2,
                                  4,
                                  GL_FLOAT,
                                  false,
                                  32,
                                  (void*) 16);

            // glEnableVertexAttribArray(2);
            // glBindBuffer(GL_ARRAY_BUFFER, circle_c_buffer);
            // glBufferData(GL_ARRAY_BUFFER, N_MAX_CIRCLES*4*sizeof(circles[0]), NULL, GL_STREAM_DRAW);
            // glBufferSubData(GL_ARRAY_BUFFER, 0, n_circles*16, (void*) circles);
            // glVertexAttribPointer(2,
            //                       4,
            //                       GL_FLOAT,
            //                       false,
            //                       32,
            //                       (void*) 16);

            glVertexAttribDivisor(0, 0);
            glVertexAttribDivisor(1, 1);
            glVertexAttribDivisor(2, 1);
            glDrawArraysInstanced(GL_POLYGON, 0, vi_circle.n_index_buffer, n_circles);
        }
    }

    return 0;
}

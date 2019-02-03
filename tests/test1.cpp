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

    init_memory(); //TODO: figure out what the user actually needs to specify, and make everything else happen automatically
    window_t wnd = create_window("Test Window", "test window");

    load_default_programs();

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

    auto circle_draw = make_circle_drawing_buffer(N_MAX_CIRCLES, 3+2+3); //3 pos, 2 radius, 3, color
    GLuint circle_buffer = circle_draw.gl_buffer;
    GLuint index_buffer = circle_draw.gl_buffer;
    circles = circle_draw.data_buffer;

    // bind_vertex_and_index_buffers(vi_circle);

    real dt = 0.1;
    while(update_window(wnd))
    {
        {
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
            glUseProgram(pinfo_circle_2d.program);

            real transform[] = {
                1, 0, 0,
                0, 1, 0,
                0, 0, 1,
            };
            glUniformMatrix3fv(pinfo_circle_2d.uniforms[0], 1, true, transform);

            n_circles = 0;
            // glBegin(GL_TRIANGLES);
        }

        real energy = 0;
        t += dt;
        for(int i = 0; i < n_ps; i++)
        {
            real tau = i%5;// *2*pi/N_MAX_PARTICLES;
            real omega_color = 2*pi/5;

            real old_energy = dot(ps[i].v, ps[i].v);

            ps[i].r += dt*ps[i].v;
            ps[i].v += dt*(
                (1.0 - 1*sqrt(dot(ps[i].r, ps[i].r)))*(real2){ps[i].v.y, -ps[i].v.x}
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
                ps[i_spawn].v *= 0.5;//1.0-0.1*(rand()%100)*(1/100.0f);
                ps[i_spawn].v += {speed*cos(theta), speed*sin(theta)};
            }

            draw_circle(wnd,
                        0.2*ps[i].r.x, 0.2*ps[i].r.y,
                        0.002,
                        cos(omega_color*tau), cos(omega_color*(tau+pi*2/3)), cos(omega_color*(tau+pi*4/3)));
        }

        if(next_ps > n_ps) n_ps = next_ps;

        draw_circle(wnd, 0, 0, 0.05, 1, 1, 1);
        // for(float theta = 0; theta < 2*pi*10; theta += 2*pi/50) draw_circle(wnd, theta/10-1, sin(t+theta)/10, 0.005, 1, 1, 1);

        end_draw_circles(N_MAX_CIRCLES, n_circles, circle_buffer, index_buffer, circles);
    }

    return 0;
}

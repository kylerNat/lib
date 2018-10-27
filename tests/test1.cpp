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

    int N_MAX_PARTICLES = 10000;
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

            real red = 1.0, green = 1.0, blue = 1.0;
            glUniform3f(pinfo_2d_flat.uniforms[1], red, green, blue);
            // glBegin(GL_TRIANGLES);
        }

        t += dt;
        for(int i = 0; i < n_ps; i++)
        {
            real tau = i*2*pi/N_MAX_PARTICLES;
            real omega_color = 10;

            ps[i].r += dt*ps[i].v;
            ps[i].v += dt*(
                (1.0 - 1.0*sqrt(dot(ps[i].r, ps[i].r)))*(real2){ps[i].v.y, -ps[i].v.x}
                );
            ps[i].v *= 0.99999;

            if(next_ps < N_MAX_PARTICLES)
            { //spawn new particles
                next_ps %= N_MAX_PARTICLES;
                int i_spawn = next_ps++;
                ps[i_spawn] = ps[i];
                float theta = (rand()%100)*(2.0f*pi/100.0f);
                float speed = 0.01*((rand()%100)*(0.1f/100.0f)+0.1);
                ps[i_spawn].v *= 0.5;
                ps[i_spawn].v += {speed*cos(theta), speed*sin(theta)};
            }

            draw_circle(wnd,
                        0.2*ps[i].r.x, 0.2*ps[i].r.y,
                        0.005,
                        cos(omega_color*tau), cos(omega_color*(tau+pi*2/3)), cos(omega_color*(tau+pi*4/3)));
        }
        if(next_ps > n_ps) n_ps = next_ps;
        // draw_circle(wnd, 0, 0, 0.05, 1, 1, 1);

        // glEnd();
    }

    return 0;
}

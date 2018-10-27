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

    RECT wnd_rect;
    GetWindowRect(wnd.hwnd, &wnd_rect);
    auto window_width = wnd_rect.right-wnd_rect.left;
    auto window_height = wnd_rect.bottom-wnd_rect.top;
    uint32 * bitmap = (uint32*) calloc(window_width*window_height, sizeof(uint32));
    particle* particles = (particle*) malloc(2*N_MAX_PARTICLES*sizeof(particles));
    int n_particles = 0;
    int next_particle = 0;

    int n_twirlies = 5;
    for(int i = 0; i < n_twirlies; i++)
    {
        float r = 100;
        float s = 100;
        float x = cos(i*2*pi/n_twirlies);
        float y = sin(i*2*pi/n_twirlies);
        particles[n_particles++] = {r*x, r*y,
                                    s*y, -s*x};
    }
    next_particle = n_particles;

    // srand(time(NULL));

    real dt = 0.01;
    while(update_window(wnd))
    {
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

            // draw_circle(wnd,
            //             0.2*ps[i].r.x, 0.2*ps[i].r.y,
            //             0.005,
            //             cos(omega_color*tau), cos(omega_color*(tau+pi*2/3)), cos(omega_color*(tau+pi*4/3)));
        }
        if(next_ps > n_ps) n_ps = next_ps;
        draw_circle(wnd, 0, 0, 0.05, 1, 1, 1);

        {
            memset(bitmap, 0, window_width*window_height*sizeof(bitmap[0]));

            for(int p = 0; p < n_ps; p++)
            {
                int x = window_width/2+100*ps[p].r.x;
                int y = window_height/2+100*ps[p].r.y;

                if(x < 0 || x >= window_width ||
                   y < 0 || y >= window_height) continue;
                bitmap[x+window_width*y] |= 0x00FFFF;
            }

            for(int p = 0; p < n_particles; p++)
            {
                int x = window_width/2+particles[p].r.x;
                int y = window_height/2+particles[p].r.y;

                particles[p].r += dt*particles[p].v;

                particles[p].v += dt*(1.0-0.01*sqrt(dot(particles[p].r, particles[p].r)))*(real2){particles[p].v.y, -particles[p].v.x};
                particles[p].v *= 0.99999;

                // float theta = (rand()%100)*(2.0f*pi/100.0f);
                // float speed = (rand()%100)*(2.0f/100.0f)+0.1;
                // particles[p].v += dt*(real2){speed*cos(theta), speed*sin(theta)};

                if(next_particle < N_MAX_PARTICLES)
                { //spawn new particles
                    next_particle %= N_MAX_PARTICLES;
                    int p_spawn = next_particle++;
                    particles[p_spawn] = particles[p];
                    float theta = (rand()%100)*(2.0f*pi/100.0f);
                    float speed = (rand()%100)*(0.1f/100.0f)+0.1;
                    particles[p_spawn].v *= 0.5;
                    particles[p_spawn].v += {speed*cos(theta), speed*sin(theta)};
                }

                if(x < 0 || x >= window_width ||
                   y < 0 || y >= window_height) continue;
                bitmap[x+window_width*y] |= 0xFF0000;
            }
            if(next_particle > n_particles) n_particles = next_particle;

            {
                BITMAPINFO bmi;
                bmi.bmiHeader.biSize = sizeof(bmi.bmiHeader);
                bmi.bmiHeader.biWidth = window_width;
                bmi.bmiHeader.biHeight = -window_height;
                bmi.bmiHeader.biPlanes = 1;
                bmi.bmiHeader.biBitCount = 32;
                bmi.bmiHeader.biCompression = BI_RGB;

                StretchDIBits(GetDC(wnd.hwnd),
                              (wnd_rect.right-wnd_rect.left-window_width)/2, (wnd_rect.bottom-wnd_rect.top-window_height)/2,
                              window_width, window_height, //destination
                              0, 0, window_width, window_height, //source
                              bitmap,
                              &bmi,
                              DIB_RGB_COLORS,
                              SRCCOPY);
            }
        }
    }

    // while(update_window(wnd))
    // {
    //     t += 0.0005;
    //     int n_dots = 1280;
    //     for(int i = 0; i < n_dots; i++)
    //     {
    //         real tau = t+i*2*pi/n_dots;
    //         real omega_fast = 1+64;
    //         real omega_color = 10;
    //         real r_1 = 0.5, r_2 = 0.1;
    //         real wiggly = 0.1*cos(100*t);
    //         real omega_wiggle = 5;
    //         draw_circle(wnd,
    //                     r_1*cos(tau+wiggly*sin(omega_wiggle*tau))+r_2*cos(tau*omega_fast),
    //                     r_1*sin(tau+wiggly*sin(omega_wiggle*tau))+r_2*sin(tau*omega_fast),
    //                     0.01,
    //                     cos(omega_color*tau), cos(omega_color*(tau+pi*2/3)), cos(omega_color*(tau+pi*4/3)));
    //     }
    //     draw_circle(wnd, 0, 0, 0.05, 1, 1, 1);
    // }

    return 0;
}

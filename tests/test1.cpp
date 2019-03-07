#include <ui/win32_gl_window.h>
#include <maths/maths.h>

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
    gl_init_general_buffers(); //TODO: put this somewhere else for better platform independence

    log_output("created window\n");
    show_window(wnd);
    log_output("showing window\n");

    real t = 0;

    const real max_radius = 5;
    const real initial_radius = max_radius/2;
    int N_MAX_PARTICLES = 1000000;
    particle* ps = (particle*) permalloc(N_MAX_PARTICLES*sizeof(particle));
    int n_ps = 100000;//N_MAX_PARTICLES;
    int next_ps = n_ps;

    //initial conditions
    for(int i = 0; i < n_ps; i++)
    {
        real theta = i*2*pi/n_ps;//i*pi*(3-sqrt(5))/n_ps;
        real r = initial_radius*pow(((real)i)/n_ps, 1.0/3.0);
        real p = 0.0;
        ps[i] = {r*cos(theta), r*sin(theta), p*sin(theta), -p*cos(theta)};
    }

    // srand(time(NULL));

    #define N_MAX_CIRCLES 10000000
    circle_render_info* circles = (circle_render_info*) permalloc(N_MAX_CIRCLES*sizeof(circle_render_info));
    int n_circles;

    real dt = 0.1;
    while(update_window(wnd))
    {
        n_circles = 0;

        real energy = 0;
        t += dt;
        #define n_density_bins 10000
        int densities[n_density_bins] = {};
        #define calc_bin_id(rsq) clamp((n_density_bins+n_density_bins/16*log(rsq/sq(max_radius))), 0, n_density_bins)
        for(int i = 0; i < n_ps; i++)
        {
            real rsq = dot(ps[i].r, ps[i].r);
            int bin_id = calc_bin_id(rsq);
            densities[bin_id]++;
        }
        for(int i = 1; i < n_density_bins; i++)
        {
            densities[i] += densities[i-1];
        }
        for(int i = 0; i < n_ps; i++)
        {
            real rsq = dot(ps[i].r, ps[i].r);
            int bin_id = calc_bin_id(rsq)-1;

            real tau = i%5;// *2*pi/N_MAX_PARTICLES;
            real omega_color = 2*pi/5;

            real old_energy = dot(ps[i].v, ps[i].v);

            ps[i].r += dt*ps[i].v;
            //apply gravity
            const real G = 0.001;
            if(bin_id > 0)
                ps[i].v += dt*(-(G*densities[bin_id]/n_ps)*pow(rsq, -1.5)*ps[i].r);
            else if(dot(ps[i].v, ps[i].r) < 0)
            {
                // //cloud has fully collapsed
                // log_output("collapse time: ", t); //138.599258, 138.999283, depending on collapse detection threshold
                // log_output("\n calculated collapse time: ", pi/2*pow(initial_radius, 1.5)/sqrt(2*G*1)); //138.8400087, it works!!!
                // return 0;
                ps[i].v *= -1;
            }
            // ps[i].v += dt*(
            //     (1.0 - 1*sqrt(dot(ps[i].r, ps[i].r)))*(real2){ps[i].v.y, -ps[i].v.x}
            //     );

            // energy = dot(ps[i].v, ps[i].v);

            // ps[i].v *= sqrt(old_energy/energy);//0.9999;

            // if(next_ps < N_MAX_PARTICLES)
            // { //spawn new particles
            //     next_ps %= N_MAX_PARTICLES;
            //     int i_spawn = next_ps++;
            //     ps[i_spawn] = ps[i];
            //     real theta = (rand()%100)*(2.0f*pi/100.0f);
            //     real speed = 0.001*((rand()%100)*(0.1f/100.0f)+0.1);
            //     ps[i_spawn].v *= 0.5;//1.0-0.1*(rand()%100)*(1/100.0f);
            //     ps[i_spawn].v += {speed*cos(theta), speed*sin(theta)};
            // }

            circles[n_circles++] = {
                // {0.2*sqrt(rsq), 20*0.2*sqrt(dot(ps[i].v, ps[i].v)), 0},
                {0.2*ps[i].r.x, 0.2*ps[i].r.y, 0},
                0.002+bin_id > 0 ? 0.02*densities[bin_id]/n_ps: 0,
                {50*ps[i].v.x, 50*ps[i].v.y, densities[bin_id]*1.0/n_ps, 1}
            };
        }

        if(next_ps > n_ps) n_ps = next_ps;

        // circles[n_circles++] = {
        //     {0, 0, 0},
        //     0.05,
        //     {1, 1, 1, 1}
        // };
        // for(real theta = 0; theta < 2*pi*10; theta += 2*pi/50) draw_circle(wnd, theta/10-1, sin(t+theta)/10, 0.005, 1, 1, 1);

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        draw_circles(circles, n_circles);
    }

    return 0;
}

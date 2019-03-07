struct circle_render_info
{
    real3 X;
    real r;
    real4 color;
};

struct cloud_render_info
{
    int16* density; //cloud density in the some basis, integer values normalized to [-1, 1], -2^15 is clamped to -1
};

#ifndef MATHS
#define MATHS

#include <math.h>
#include <utils/misc.h>
#include <smmintrin.h>

#define pi (3.14159265358979323846264338327950)
#define e (2.718281828)

#define sq(a) ((a)*(a))

typedef float real;

#define sqrt(a) sqrtf(a)

//float instead real because this only works for 32 bit floating point values
float invsqrt(float a)
{
    int isqrti;
    float isqrt = a;

    isqrti = *(int*) &isqrt;
    isqrti = 0x5f375a86 - (isqrti >> 1); //magic

    isqrt = *(float*) &isqrti;
    isqrt *= (1.5f - (a * 0.5f * isqrt * isqrt));
    isqrt *= (1.5f - (a * 0.5f * isqrt * isqrt)); //second itteration, uncomment if greater accuracy is needed
    return isqrt;
}

inline float32 abs(float32 x)//if it inlines correctly this should be 1 op
{
    //const __m128i sign_mask = _mm_set_epi32(0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0x7FFFFFFF);
    __m128 absolute_value = _mm_set_ss(x);
    absolute_value = _mm_and_ps(absolute_value, ((__m128) _mm_set_epi32(0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0x7FFFFFFF)));//set the sign bit to 0 to get the absolute value

    float32 out;
    _mm_store_ss(&out, absolute_value);
    return out;
}

inline float32 floor(float32 x)
{
    __m128 floor_x = _mm_set_ss(x);

    //floor_x = _mm_round_ps(floor_x, _MM_FROUND_TO_NEG_INF);//TODO: use roundps when available
    floor_x = _mm_cvtepi32_ps(_mm_cvttps_epi32(floor_x));

    float32 out;
    _mm_store_ss(&out, floor_x);
    return out;
}

//TODO: more accurate trig functions?

inline float32 fast_cos(float32 x)
{//TODO: optimize!(more)
    x = abs(x);
    x -= floor(x);
    x -= 0.5;
    x = abs(x);
    x -= 0.25;
    //x is converted to a triangle wave since the polynomial approxomation only works between -pi/4 and pi/4 radians
    float32 out = -32.0;
    out *= x;
    out *= x;
    out += 6.0;
    out *= x;
    return out;//x*(6.0-32.0*x*x);
}

inline float32 fast_sin(float32 x)
{
    return fast_cos(x - 0.25);
}
//macro to define simultaneous assignment and equality operators (+=, *=, etc.)
#define eq_op(op, rtype, atype, btype) rtype operator##op##=(atype& a, btype b){return a=a op b;};

//TODO: make way to specify custom procedures for specific optimization

// #define type real
// #define N 2
// #include "math/generic_vector.h"

// #define type real
// #define N 2
// #include "math/generic_square_matrix.h"

// #define type real
// #define N 3
// #include "math/generic_vector.h"

// #define type real
// #define N 4
// #include "math/generic_vector.h"

// #define type real
// #define N 3
// #include "math/generic_square_matrix.h"

// #define type real
// #define N 4
// #include "math/generic_square_matrix.h"

// #define type real
// #define N 2
// #define M 3
// #include "math/generic_rectangular_matrix.h"

// #define type real
// #define N 50
// #include "math/generic_vector.h"

// #define type real
// #define N 50
// #include "math/generic_square_matrix.h"

// #define type real
// #define N 4
// #define x0 r
// #define x1 i
// #define x2 j
// #define x3 k
// #define typeN quaternion
// #include "math/generic_vector.h"

// real3 cross(real3 a, real3 b)
// {
//     return {a.y*b.z-a.z*b.y,
//             a.z*b.x-a.x*b.z,
//             a.x*b.y-a.y*b.x};
// }

// /* multiplies two 2d vectors in R^2 as if they were complex numbers
//  * i.e. the result is a vector with the magnitude |a||b|,
//  * and an angle with the x axis that is the sum of the original angles*/
// real2 rotation_multiply(real2 a, real2 b)
// {
//     return {a.x*b.x-a.y*b.y,
//             a.x*b.y+a.y*b.x};
// }

// quaternion operator*(quaternion a, quaternion b)
// {
//     return {a.r*b.r-a.i*b.i-a.j*b.j-a.k*b.k,
//             a.r*b.i+a.i*b.r+a.j*b.k-a.k*b.j,
//             a.r*b.j+a.j*b.r+a.k*b.i-a.i*b.k,
//             a.r*b.k+a.k*b.r+a.i*b.j-a.j*b.i};
// }

#endif

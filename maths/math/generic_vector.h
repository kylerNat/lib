/* abusing the preprocessor because we can
 *
 * including this file defines vectors of type type and size N,
 */

#ifndef x0
#define x0 x
#endif
#ifndef x1
#define x1 y
#endif
#ifndef x2
#define x2 z
#endif
#ifndef x3
#define x3 w
#endif

#ifndef typeN
#define typeN CONCAT(type, N)
#endif

struct typeN
{
    union
    {
        #if N <= 4
        struct
        {
            type x0;
            type x1;
            #if N > 2
            type x2;
            #if N > 3
            type x3;
            #endif
            #endif
        };
        #endif
        type data[N];
    };

    type& operator[](int i)
    {
        return data[i];
    }
};

typeN operator*(type a, typeN x)
{
    typeN ax;
    for(int i = 0; i < N; i++) ax[i] = a*x[i];
    return ax;
}

typeN operator*(typeN x, type a)
{
    return a*x;
}

typeN operator/(typeN x, type a)
{
    return (1.0/a)*x;
}

typeN operator+(typeN a, typeN b)
{
    typeN sum;
    for(int i = 0; i < N; i++) sum[i] = a[i]+b[i];
    return sum;
}

typeN operator-(typeN a, typeN b)
{
    typeN sum;
    for(int i = 0; i < N; i++) sum[i] = a[i]-b[i];
    return sum;
}

type dot(typeN a, typeN b)
{
    type product = 0;
    for(int i = 0; i < N; i++) product += a[i]*b[i];
    return product;
}

type norm(typeN a)
{
    return sqrt(dot(a, a));
}

typeN normalize(typeN a)
{
    return invsqrt(dot(a, a))*a;
}

eq_op(+, typeN, typeN, typeN);
eq_op(-, typeN, typeN, typeN);
eq_op(*, typeN, typeN, type);
eq_op(/, typeN, typeN, type);

#undef typeN
#undef type
#undef N
#undef x0
#undef x1
#undef x2
#undef x3

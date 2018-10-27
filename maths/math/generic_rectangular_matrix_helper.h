#define typeR CONCAT(type, R)
#define typeC CONCAT(type, C)

#ifndef typeRxC
#define typeRxC CONCAT(CONCAT(type, R), CONCAT(x, C))
#endif
#ifndef typeCxR
#define typeCxR CONCAT(CONCAT(type, C), CONCAT(x, R))
#endif

typeRxC operator*(type c, typeRxC A)
{
    typeRxC cA;
    for(int i = 0; i < R*C; i++) cA.data[i] = c*A.data[i];
    return cA;
}

typeRxC operator*(typeRxC A, type c)
{
    return c*A;
}

typeRxC operator/(typeRxC A, type c)
{
    return (1.0/c)*A;
}

typeRxC operator+(typeRxC A, typeRxC B)
{
    typeRxC sum;
    for(int i = 0; i < R; i++) sum[i] = A[i]+B[i];
    return sum;
}

typeRxC operator-(typeRxC A, typeRxC B)
{
    typeRxC sum;
    for(int i = 0; i < R; i++) sum[i] = A[i]-B[i];
    return sum;
}

eq_op(+, typeRxC, typeRxC, typeRxC);
eq_op(-, typeRxC, typeRxC, typeRxC);
eq_op(*, typeRxC, typeRxC, type);
eq_op(/, typeRxC, typeRxC, type);

typeCxR transpose(typeRxC A)
{
    typeCxR At;
    for(int i = 0; i < R; i++)
        for(int j = 0; j < C; j++)
            At[j][i] = A[i][j];
    return At;
}

typeR operator*(typeRxC A, typeC x)
{
    typeR result = {};
    for(int i = 0; i < R; i++) result += A[i]*x[i];
    return result;
}

#undef typeR
#undef typeC
#undef typeRxC
#undef typeCxR
#undef R
#undef C

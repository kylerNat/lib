/* including this file defines matrices of type type and size NxN,
 */

/*TODO: as needed, diagonalization/jordanlization, exponential, finding eigenvectors, any other operations*/

#define typeN CONCAT(type, N)

#ifndef typeNxN
#define typeNxN CONCAT(CONCAT(type, N), CONCAT(x, N))
#endif

struct typeNxN
{
    union
    {
        struct
        {
            typeN columns[N];
        };
        type data[N*N];
    };

    typeN& operator[](int i)
    {
        return columns[i];
    }
};

#define identityN CONCAT(identity, N)
typeNxN identityN(int lambda)
{
    typeNxN Lambda = {};
    for(int i = 0; i < N; i++) Lambda[i][i] = lambda;
    return Lambda;
}

typeNxN operator*(type c, typeNxN A)
{
    typeNxN cA;
    for(int i = 0; i < N*N; i++) cA.data[i] = c*A.data[i];
    return cA;
}

typeNxN operator*(typeNxN A, type c)
{
    return c*A;
}

typeNxN operator/(typeNxN A, type c)
{
    return (1.0/c)*A;
}

typeNxN operator+(typeNxN A, typeNxN B)
{
    typeNxN sum;
    for(int i = 0; i < N; i++) sum[i] = A[i]+B[i];
    return sum;
}

typeNxN operator-(typeNxN A, typeNxN B)
{
    typeNxN sum;
    for(int i = 0; i < N; i++) sum[i] = A[i]-B[i];
    return sum;
}

typeNxN transpose(typeNxN A)
{
    typeNxN At;
    for(int i = 0; i < N; i++)
        for(int j = 0; j < N; j++)
            At[j][i] = A[i][j];
    return At;
}

typeN operator*(typeNxN A, typeN x)
{
    typeN result = {};
    for(int i = 0; i < N; i++) result += A[i]*x[i];
    return result;
}

typeN operator*(typeN x, typeNxN A)
{
    typeNxN At = transpose(A);
    return At*x;
}

typeNxN operator*(typeNxN A, typeNxN B)
{
    typeNxN product;
    for(int i = 0; i < N; i++) product[i] = A*B[i];
    return product;
}

eq_op(+, typeNxN, typeNxN, typeNxN);
eq_op(-, typeNxN, typeNxN, typeNxN);
eq_op(*, typeNxN, typeNxN, typeNxN);
eq_op(*, typeNxN, typeNxN, type);
eq_op(/, typeNxN, typeNxN, type);

type tr(typeNxN A)
{
    type trace = 0;
    for(int i = 0; i < N; i++) trace += A[i][i];
    return trace;
}

type det(typeNxN A)
{
    type numerator = 1;
    type denominator = 1;

    //use column operations to make A an upper triagular matrix
    for(int i = 0; i < N-1; i++)
        for(int j = i+1; j < N; j++)
        {
            A[j] *= A[i][i];
            denominator *= A[i][i];
            A[j] -= A[j][i]*A[i];
        }

    //take the determinant of the upper tringular matrix
    for(int i = 0; i < N; i++) numerator *= A[i][i];

    return numerator/denominator;
}

//find the inverse of a matrix using Gauss-Jordan elimination
typeNxN inverse(typeNxN A)
{
    typeNxN Ai = identityN(1);

    //reduce to upper triagular matrix
    for(int i = 0; i < N-1; i++)
        for(int j = i+1; j < N; j++)
        {
            A[j] *= A[i][i];
            Ai[j] *= A[i][i];

            A[j] -= A[j][i]*A[i];
            Ai[j] -= A[j][i]*Ai[i];
        }

    //reduce to diagional matrix
    for(int i = N-1; i >= 0; i--)
        for(int j = i-1; j >= 0; j--)
        {
            A[j] *= A[i][i];
            Ai[j] *= A[i][i];

            A[j] -= A[j][i]*A[i];
            Ai[j] -= A[j][i]*Ai[i];
        }

    //reduce to identity matrix
    for(int i = 0; i < N; i++)
    {
        //we don't need to update A anymore, since we will just throw it out after this step
        Ai[i] /= A[i][i];
    }

    return Ai;
}

#undef identityN
#undef typeNxN
#undef typeN
#undef type
#undef N

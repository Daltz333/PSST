/**
 * Computes the inverse mod between input A and M
*/
int modInverse(int A, int M)
{
    for (int X = 1; X < M; X++)
        if (((A % M) * (X % M)) % M == 1)
            return X;

    return -1;
}

/** Safe mod function */
/** https://math.stackexchange.com/questions/195634/how-do-you-calculate-the-modulo-of-a-high-raised-number */
long long int modfun(long long int a, long long int b, long long int mod)
{
    long long int result = 1;
    while (b > 0)
    {
        if (b & 1)
        {
            a=a%mod;
            result = (result * a)%mod;
            result=result%mod;
        }

        b=b>>1;
        a=a%mod;
        a = (a*a)%mod;
        a=a%mod;
    }

    return result;
}

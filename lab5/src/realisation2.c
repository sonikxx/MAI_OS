#include "realisation.h"

void swap_int(int* x, int* y) {
    int tmp = *x;
    *x = *y;
    *y = tmp;
}

int GCD(int x, int y) {
    if (x > y) {
        swap_int(&x, &y);
    }
    for (int i = x; i > 1; --i) {
        if (x % i == 0 && y % i == 0) {
            return i;
        }
    }
    return 1;
}

int PrimeCount(int A, int B) {
    if (A < 2) A = 2;
    int res = 0;
    char sieve [SQRT_OF_MAXINT];
    for (int i = 0; i < SQRT_OF_MAXINT; ++i) sieve[i] = 1;
    long long primes[SQRT_OF_MAXINT];
    int t = 0;
    for(long long i = 2; i < SQRT_OF_MAXINT; ++i) {
        if (sieve[i] == 0) continue;
        primes[t++] = i;
        for(long long j = i * i; j < SQRT_OF_MAXINT; j += i)
            sieve[j] = 0;
    }
    int flag;
    for (long long i = A; i <= B; ++i) {
        flag = 1;
        for (int j = 0; j < t && primes[j]*primes[j] <= i; ++j) {
            if (i % primes[j] == 0) {
                flag = 0;
                break;
            }
        }
        res += flag;
    }
    return res;
}
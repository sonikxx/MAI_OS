#include "realisation.h"
#include <stdio.h>

#define check_ok(VALUE, OKVAL, MSG) if (VALUE != OKVAL) { printf("%s", MSG); return 1; }

int main(int argc, const char** argv) {
    int q;
    int x, y;
    int A, B;
    printf("Enter query: 1) get GCD(x,y)  2) count primes between A, B\n");
    while (scanf("%d", &q) > 0) {
        switch (q) {
            case 1:
                printf("enter integer x, y: ");
                check_ok(scanf("%d%d", &x, &y), 2, "Error reading integer!\n");
                printf("GCD(%d, %d) = %d\n", x, y, GCD(x, y));
                break;
            case 2:
                printf("enter integer A, B: ");
                check_ok(scanf("%d %d", &A, &B), 2, "Error reading integers!\n");
                printf("There are %d primes between %d and %d\n", PrimeCount(A, B), A, B);
                break;
            default:
                printf("End.\n");
                return 0;
        }
    }
}
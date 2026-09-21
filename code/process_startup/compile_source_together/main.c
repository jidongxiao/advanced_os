#include <stdio.h>
#include "math_utils.h"

int main(void)
{
    int a = 6;
    int b = 7;

    printf("add(%d, %d) = %d\n", a, b, add(a, b));
    printf("multiply(%d, %d) = %d\n", a, b, multiply(a, b));

    return 0;
}

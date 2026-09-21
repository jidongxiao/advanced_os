#include <stdio.h>
#include <stdlib.h>

int main(void) {
    void *p1 = malloc(64);
    void *p2 = malloc(128);
    printf("Allocated memory\n");
    free(p1);
    free(p2);
    printf("Freed memory\n");
    return 0;
}

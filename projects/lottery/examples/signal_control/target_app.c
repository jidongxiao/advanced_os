#include <stdio.h>
#include <unistd.h>

int main(void)
{
    int count = 0;
    printf("[TARGET] Running with PID: %d\n", getpid());

    while (1) {
        printf("[TARGET] Active loop tick %d...\n", ++count);
        usleep(200000); /* 200ms */
    }

    return 0;
}

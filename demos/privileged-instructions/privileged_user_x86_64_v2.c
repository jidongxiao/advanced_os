#define _GNU_SOURCE

#include <stdio.h>
#include <signal.h>
#include <ucontext.h>

void sigsegv_handler(int sig, siginfo_t *info, void *context)
{
    ucontext_t *uc = (ucontext_t *)context;

    printf("\nSIGSEGV caught!\n");
    printf("The CPU prevented the privileged instruction from executing.\n");
    printf("Skipping the instruction and continuing...\n");

    /*
     * On x86-64, the instruction:
     *
     *     mov %cr3, %rax
     *
     * is 3 bytes long.
     *
     * RIP points to the instruction that caused the exception.
     * Advance RIP by 3 bytes so execution resumes at the
     * instruction following mov %cr3, %rax.
     */
    uc->uc_mcontext.gregs[REG_RIP] += 3;
}

int main(void)
{
    unsigned long value;

    /*
     * Install a handler for SIGSEGV.
     */
    struct sigaction sa;

    // Specify the function that will be called when SIGSEGV occurs.
    sa.sa_sigaction = sigsegv_handler;
    // Initialize the signal mask to empty:
    // do not block any additional signals while the handler runs.
    sigemptyset(&sa.sa_mask);

    // the handler will use the extended three-argument interface
    sa.sa_flags = SA_SIGINFO;

    // Install the SIGSEGV handler.
    // From now on, when this process receives SIGSEGV,
    // sigsegv_handler() will be called instead of the default action.
    if (sigaction(SIGSEGV, &sa, NULL) == -1) {
        perror("sigaction");
        return 1;
    }

    printf("Trying to read CR3 from user space...\n");

    asm volatile(
        "mov %%cr3, %0"
        : "=r"(value)
    );

    printf("CR3 = %lx\n", value);

    printf("The program continued after the privileged instruction.\n");

    return 0;
}

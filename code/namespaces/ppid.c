#define _GNU_SOURCE
#include <sched.h>      // clone(), CLONE_NEWPID
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>   // waitpid()
#include <unistd.h>     // getpid()

// clone() does not automatically allocate a stack for the child.
// We provide a 1 MB stack for the child process.
static char child_stack[1048576];

static int child_fn()
{
    // The child is the first process created in the new PID namespace.
    // Therefore, it is assigned PID 1 inside that namespace.
    // But what is the parent ID?
    printf("PID: %ld\n", (long)getppid());
    return 0;
}

int main()
{
    /*
     * Create a new process.
     *
     * child_fn:
     *     Function executed by the child process.
     *
     * child_stack + 1048576:
     *     Top of the 1 MB stack provided for the child.
     *
     *     We pass the TOP of the stack because stacks on
     *     architectures such as x86-64 grow downward.
     *
     *     This is different from fork(): fork() hides the
     *     stack setup from us, while this clone() interface
     *     expects us to provide the child's stack.
     *
     * CLONE_NEWPID:
     *     Put the child into a new PID namespace.
     *
     * SIGCHLD:
     *     Send SIGCHLD to the parent when the child terminates.
     *
     * NULL:
     *     No argument is passed to child_fn().
     *
     * IMPORTANT:
     *     The parent and child see different PIDs for the same process.
     *
     *     Parent's PID namespace:  child = e.g. 48687
     *     Child's PID namespace:   child = 1
     */
    pid_t child_pid = clone(
        child_fn,
        child_stack + 1048576,
        CLONE_NEWPID | SIGCHLD,
        NULL
    );

    /*
     * CLONE_NEWPID is a privileged operation.
     *
     * If the program is run by an ordinary user without the
     * required privilege, clone() fails and returns -1.
     */
    if (child_pid == -1) {
        perror("clone");
        return 1;
    }

    /*
     * clone() returns the child's PID as seen from the
     * parent's PID namespace.
     *
     * For example:
     *
     *     clone() = 48687
     *
     * This does NOT mean the child sees itself as PID 48687.
     * Inside its new PID namespace, the child is PID 1.
     */
    printf("clone() = %ld\n", (long)child_pid);

    /*
     * Wait for the child to terminate.
     *
     * The parent refers to the child using its PID in the
     * parent's PID namespace (e.g. 48687).
     */
    waitpid(child_pid, NULL, 0);
    return 0;
}

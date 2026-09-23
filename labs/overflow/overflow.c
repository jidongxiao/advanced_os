/*
 * Compile this program with:
 * gcc -fno-stack-protector -Wno-stringop-overflow -o overflow overflow.c -g
 *
 * To run this program, you may need to disable ASLR
 * (Address Space Layout Randomization):
 *
 * sudo sysctl -w kernel.randomize_va_space=0
 *
 * You can restore ASLR afterward with:
 *
 * sudo sysctl -w kernel.randomize_va_space=2
 *
 * Useful GDB commands:
 *
 * (gdb) info registers pc
 *     Shows the current value of the program counter.
 *
 * (gdb) ni
 *     Executes the next assembly instruction.
 *
 * (gdb) x/20x buf
 *     Examines 20 hexadecimal units of memory starting
 *     at the address of buf.
 *
 * (gdb) disas main
 *     Displays the assembly instructions for main().
 */

#include <stdio.h>  /* for printf() */
#include <stdlib.h> /* for exit() */
#include <string.h>

int your_fcn(void) {
        char buf[5];

        strcpy(buf, "aaaa");

        return 0;
}

/*
 * To examine the assembly code of main(), run:
 *
 * (gdb) disas main
 */
int main(void) {
        int mine = 0;
        int yours = 0;
        yours = your_fcn();
        mine = yours + 1;
        if (mine > yours)
                printf("You lost!\n");
        else
                printf("You won!\n");
        exit(0);
}

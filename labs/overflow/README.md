# Overview

In this lab, you will explore how a **buffer overflow vulnerability** can corrupt data on the stack and alter a program's behavior. You will modify a vulnerable C program to make it print `"You won!"`, even though the program's logic is designed to print `"You lost!"` under normal circumstances. You will use GDB to inspect the stack, examine memory contents, and understand how overflowing a buffer can affect nearby variables.

# Specification

The program determines whether you win based on the following logic:

```c
mine = yours + 1;

if (mine > yours)
        printf("You lost!\n");
else
        printf("You won!\n");
```

Under normal circumstances, `mine` is greater than `yours`, so the program prints `"You lost!"`. Your goal is to use a buffer overflow to alter the program's behavior so that the program prints "You won!".

## Requirements

* **Do not modify the `main()` function.**
* You may modify `your_fcn()` as needed, but you must exploit a **buffer overflow vulnerability**, not an integer overflow vulnerability.
* Set the size of the buffer to the **last two digits of your RIN number**. This ensures that each student's solution uses a different buffer size.
* Modify the `strcpy()` statement so that the program prints `"You won!"`.
* Hardcoding values is permitted.
* You may use GDB to inspect the stack and determine how the overflow affects nearby memory.

### Example

If your RIN is `662 123 456`, the last two digits are `56`. Therefore, your buffer should be declared as:

```c
char buf[56];
```

You should then modify the `strcpy()` statement and its input string so that the program still prints:

```text
You won!
```

# Program

The starting program is provided below:

```c
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

        /*
         * To understand why we copy this string into buf,
         * use GDB to examine the memory starting at buf:
         *
         * (gdb) x/20x buf
         *
         * The first 'x' means examine memory, and the second
         * 'x' means display the contents in hexadecimal.
         */
        strcpy(buf, "aaaaaaaaaaaaa\xe2\x51\x55\x55\x55\x55");

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
```

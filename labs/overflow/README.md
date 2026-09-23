# Overview

In this lab, you will explore how a **buffer overflow vulnerability** can corrupt data on the stack and alter a program's behavior. You will modify a vulnerable C program to make it print `"You won!"`, even though the program's logic is designed to print `"You lost!"` under normal circumstances. You will use GDB to inspect the stack, examine memory contents, and understand how overflowing a buffer can affect nearby variables.

# Program

The starting program [overflow.c](overflow.c) is provided.

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

You should then modify the `strcpy()` statement and its input string so that the program prints:

```text
You won!
```

## Submission

Submit on Submitty the following two files:

- Your overflow.c file
- A screenshot showing the contents of your overflow.c file using cat, followed by the execution of the program in the same terminal window, showing that it prints "You won!".

Due Date: Oct 5th, 11:59pm. Each lab has a maximum of 5 points. Late submissions will be accepted within 3 days after the deadline, with a 1-point penalty applied to the earned grade.

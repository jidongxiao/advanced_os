/*
 * compile this program with:
 * # gcc -fno-stack-protector -Wno-stringop-overflow -o overflow overflow.c -g
 *
 * to run this program, you have to turn off ASLR (address space layout randomization using this command:
 * # sudo sysctl -w kernel.randomize_va_space=0
 * you can restore ASLR afterwards using this command:
 * # sudo sysctl -w kernel.randomize_va_space=2
 * 
 * two other useful gdb commands:
 * this command shows the assembly instruction which will be run next: (gdb) info register pc
 * this command runs the next assembly instruction: (gdb) ni
 */

#include <stdio.h> /* for printf() */
#include <stdlib.h> /* for exit() */
#include <string.h>

int your_fcn(void) {
	char buf[5];
	/* to understand why we copy this to buf, run this command in gdb:
	 * (gdb) x/20x buf
	 * this above command examines the content of the memory, 
	 * starting at the beginning address of buf, and display 20 units in total, 
	 * in the command, the first x stands for examine, and the 2nd x means displaying content in hexdecimal format.
	 * */
	strcpy(buf, "aaaaaaaaaaaaa\xe2\x51\x55\x55\x55\x55");
	return 0;
}

/* in gdb, to show the assembly code of the main function, run this command:
 * (gdb) disas main
 * */
int main(void) {

	int mine = 0; 
	int yours = 0;
	yours = your_fcn(); 
	mine = yours + 1;
	if(mine > yours)
		printf("You lost!\n");
	else
		printf("You won!\n");
	exit(0);
}

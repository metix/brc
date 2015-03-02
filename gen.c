#include <stdio.h>

#define MAX_CELLS 32768
#define MAX_NESTED_LOOPS 255

extern FILE *fasm;
extern FILE *fin;
extern void error(char *msg);

static void gen_start()
{
	fprintf(fasm,
		".section .bss\n"
		"data_area:\n"
		".fill %d\n", MAX_CELLS);

	fprintf(fasm,
		".section .text\n"
		"\n.global _start\n"
		"_start:\n");

	fprintf(fasm, "mov $data_area, %%r12\n");
}

static void gen_exit()
{
	fprintf(fasm,
		"_exit:\n"
		"\tmov $60, %%rax\n"
		"\txor %%rdi, %%rdi\n"
		"\tsyscall\n");

	fprintf(fasm,
		"read:\n"
		"\tmov $0, %%rax\n"
		"\tmov $0, %%rdi\n"
		"\tmov %%r12, %%rsi\n"
		"\tmov $1, %%rdx\n"
		"\tsyscall\n"
		"\tret\n");

	fprintf(fasm,
		"write:\n"
		"\tmov $1, %%rax\n"
		"\tmov $1, %%rdi\n"
		"\tmov %%r12, %%rsi\n"
		"\tmov $1, %%rdx\n"
		"\tsyscall\n"
		"\tret\n");
}

void generate(void)
{
	gen_start();

	int op, loop_level = 0, loop_stack[MAX_NESTED_LOOPS] = {};

	while ((op = getc(fin)) != EOF)
	{
		switch (op)
		{
			case '.':
				fprintf(fasm, "\tcall write\n");
				break;
			case ',':
				fprintf(fasm, "\tcall read\n");
				break;
			case '>':
				fprintf(fasm, "\tinc %%r12\n");
				break;
			case '<':
				fprintf(fasm, "\tdec %%r12\n");
				break;
			case '+':
				fprintf(fasm, "\tincb (%%r12)\n");
				break;
			case '-':
				fprintf(fasm, "\tdecb (%%r12)\n");
				break;
			case '[':
				loop_stack[loop_level]++;

				if (loop_level >= MAX_NESTED_LOOPS)
					error("too much nested loops");

				fprintf(fasm,
					"loop_%d_%d_start:\n"
					"\tcmpb $0, (%%r12)\n"
					"\tje loop_%d_%d_end\n",
					loop_level, loop_stack[loop_level],
					loop_level, loop_stack[loop_level]);

				loop_level++;
				break;
			case ']':
				loop_level--;

				if (loop_level < 0)
					error("wrong loop placement");

				fprintf(fasm,
				"\tcmpb $0, (%%r12)\n"
				"\tjne loop_%d_%d_start\n"
				"loop_%d_%d_end:\n\n",
					loop_level, loop_stack[loop_level],
					loop_level, loop_stack[loop_level]);
				break;
		}
	}

	if (loop_level != 0)
		error("wrong loop placement");



	gen_exit();
}

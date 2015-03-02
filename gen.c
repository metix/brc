#include <stdio.h>
#include <stdlib.h>
#include "brf.h"

static int loop_level = 0, loop_stack[MAX_NESTED_LOOPS] = {};

/* code for the entry-point, and space for the cell-array */
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

/* syscall exit(), to terminate the program successfull */
static void gen_exit()
{
	fprintf(fasm,
		"_exit:\n"
		"\tmov $60, %%rax\n"
		"\txor %%rdi, %%rdi\n"
		"\tsyscall\n");
}

static void gen_io_subroutines(void)
{
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

static void gen_out(Node *op)
{
	fprintf(fasm, "\tcall write\n");
}

static void gen_in(Node *op)
{
	fprintf(fasm, "\tcall read\n");
}

static void gen_add_ptr(Node *op)
{
	if (op->value == 1)
	{
		fprintf(fasm, "\tinc %%r12\n");
		return;
	}

	fprintf(fasm, "\tadd $%d, %%r12\n", op->value);
}

static void gen_sub_ptr(Node *op)
{
	if (op->value == 1)
	{
		fprintf(fasm, "\tdec %%r12\n");
		return;
	}

	fprintf(fasm, "\tsub $%d, %%r12\n", op->value);
}

static void gen_add(Node *op)
{
	if (op->value == 1)
	{
		fprintf(fasm, "\tincb (%%r12)\n");
		return;
	}

	fprintf(fasm, "\taddb $%d, (%%r12)\n", op->value);
}

static void gen_sub(Node *op)
{
	if (op->value == 1)
	{
		fprintf(fasm, "\tdecb (%%r12)\n");
		return;
	}

	fprintf(fasm, "\tsubb $%d, (%%r12)\n", op->value);
}

static void gen_bl(Node *op)
{
	loop_stack[loop_level]++;

	if (loop_level >= MAX_NESTED_LOOPS)
		error("fatal: too much nested loops");

	fprintf(fasm,
		"loop_%d_%d_start:\n"
		"\tcmpb $0, (%%r12)\n"
		"\tje loop_%d_%d_end\n",
		loop_level, loop_stack[loop_level],
		loop_level, loop_stack[loop_level]);

	loop_level++;
}

static void gen_br(Node *op)
{
	loop_level--;

	if (loop_level < 0)
		error("fatal: wrong loop placement");

	fprintf(fasm,
	"\tcmpb $0, (%%r12)\n"
	"\tjne loop_%d_%d_start\n"
	"loop_%d_%d_end:\n\n",
		loop_level, loop_stack[loop_level],
		loop_level, loop_stack[loop_level]);
}

static void gen_set(Node *op)
{
	fprintf(fasm, "\tmov $%d, (%%r12)\n", op->value);
}

void generate(void)
{
	debug("-> start generating asm\n");
	gen_start();

	Node *op = asl;
	while (op)
	{
		switch (op->type)
		{
			case ASL_OUT: gen_out(op); break;
			case ASL_IN: gen_in(op); break;
			case ASL_INCP: gen_add_ptr(op); break;
			case ASL_DECP: gen_sub_ptr(op); break;
			case ASL_INC: gen_add(op); break;
			case ASL_DEC: gen_sub(op); break;
			case ASL_BL: gen_bl(op); break;
			case ASL_BR: gen_br(op); break;
			case ASL_SET: gen_set(op); break;
		}

		Node *tmp = op;
		op = op->next;
		free(tmp);
	}

	if (loop_level != 0)
		error("fatal: wrong loop placement");
	gen_exit();
	gen_io_subroutines();
}

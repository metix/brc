#include <stdio.h>
#include <stdlib.h>
#include "brf.h"

/* code for the entry-point, and space for the cell-array */
static void gen_start(void)
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
static void gen_exit(void)
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

static void gen_inc_ptr(Node *op)
{
	fprintf(fasm, "\tinc %%r12\n");
}

static void gen_dec_ptr(Node *op)
{
	fprintf(fasm, "\tdec %%r12\n");
}

static void gen_add_ptr(Node *op)
{
	fprintf(fasm, "\tadd $%d, %%r12\n", op->value);
}

static void gen_sub_ptr(Node *op)
{
	fprintf(fasm, "\tsub $%d, %%r12\n", op->value);
}

static void gen_inc(Node *op)
{
	fprintf(fasm, "\tincb (%%r12)\n");
}

static void gen_dec(Node *op)
{
	fprintf(fasm, "\tdecb (%%r12)\n");
}

static void gen_add(Node *op)
{
	fprintf(fasm, "\taddb $%d, (%%r12)\n", op->value);
}

static void gen_sub(Node *op)
{
	fprintf(fasm, "\tsubb $%d, (%%r12)\n", op->value);
}

static void gen_loop_begin(Node *op, int loop_level, int loop_nr)
{
	fprintf(fasm,
		"loop_%d_%d_start:\n"
		"\tcmpb $0, (%%r12)\n"
		"\tje loop_%d_%d_end\n",
		loop_level, loop_nr,
		loop_level, loop_nr);
}

static void gen_loop_end(Node *op, int loop_level, int loop_nr)
{
	fprintf(fasm,
		"\tcmpb $0, (%%r12)\n"
		"\tjne loop_%d_%d_start\n"
		"loop_%d_%d_end:\n\n",
		loop_level, loop_nr,
		loop_level, loop_nr);
}

static void gen_set(Node *op)
{
	fprintf(fasm, "\tmov $%d, (%%r12)\n", op->value);
}

static int loop_nr;

static void generate_ast(Node *op, int loop_level)
{
	int loop_nr_tmp = loop_nr;
	while (op)
	{
		switch (op->type)
		{
			case AST_OUT: gen_out(op); break;
			case AST_IN: gen_in(op); break;
			case AST_INCP: gen_inc_ptr(op); break;
			case AST_DECP: gen_dec_ptr(op); break;
			case AST_INC: gen_inc(op); break;
			case AST_DEC: gen_dec(op); break;
			case AST_ADDP: gen_add_ptr(op); break;
			case AST_SUBP: gen_sub_ptr(op); break;
			case AST_ADD: gen_add(op); break;
			case AST_SUB: gen_sub(op); break;
			case AST_SET: gen_set(op); break;
			case AST_BLOCK:
				loop_nr++;
				gen_loop_begin(op, loop_level, loop_nr_tmp);
				generate_ast(op->child, loop_level + 1);
				gen_loop_end(op, loop_level, loop_nr_tmp);
				loop_nr_tmp++;
				break;
		}

		Node *tmp = op;
		op = op->next;
		free(tmp);
	}
}

void generate(void)
{
	debug("-> start generating asm\n");
	gen_start();

	generate_ast(ast->child, 0);

	gen_exit();
	gen_io_subroutines();
}

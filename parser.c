#include <stdio.h>
#include <stdlib.h>
#include "brf.h"

Node *asl;
static Node *last_asl;

static void push_node(int type)
{
	if (asl == NULL)
	{
		asl = calloc(1, sizeof (Node));
		asl->type = type;
		asl->value = 1;
		last_asl = asl;
		return;
	}

	Node *new = calloc(1, sizeof (Node));
	new->type = type;
	new->value = 1;
	new->previous = last_asl;
	last_asl->next = new;
	last_asl = new;
}

void parse(void)
{
	debug(	"-> start parsing\n"
		"   max-nested-loops: %d\n"
		"   max-cells: %d\n",
		MAX_NESTED_LOOPS, MAX_CELLS);

	int op, loop_level = 0,	loop_stack[MAX_NESTED_LOOPS] = {};

	while ((op = getc(fin)) != EOF)
	{
		switch (op)
		{
			case '.':
				push_node(ASL_OUT);
				break;
			case ',':
				push_node(ASL_IN);
				break;
			case '>':
				push_node(ASL_INCP);
				break;
			case '<':
				push_node(ASL_DECP);
				break;
			case '+':
				push_node(ASL_INC);
				break;
			case '-':
				push_node(ASL_DEC);
				break;
			case '[':
				if (loop_level >= MAX_NESTED_LOOPS)
					error("too much nested loops");

				loop_stack[loop_level]++;
				push_node(ASL_BL);
				loop_level++;
				break;
			case ']':
				loop_level--;
				if (loop_level < 0)
					error("wrong loop placement");
				push_node(ASL_BR);
				break;
		}
	}

	if (loop_level != 0)
		error("wrong loop placement");
}

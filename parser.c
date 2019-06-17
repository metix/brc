#include <stdio.h>
#include <stdlib.h>
#include "brf.h"

Node *ast;

static Node *create_node(int type)
{
	Node *new = calloc(1, sizeof (Node));
	new->type = type;
	if (type != AST_BLOCK)
		new->value = 1;
	return new;
}

static void push_node(Node *parent, Node *node)
{
	Node *tmp = parent->child;

	if (tmp == NULL)
	{
		parent->child = node;
		return;
	}

	// find last node in list
	while (tmp->next != NULL)
		tmp = tmp->next;

	// append node
	tmp->next = node;
}

static void parse_block(Node *n, int block_level)
{
	int op;

	while ((op = getc(fin)) != EOF)
	{
		Node *block;
		switch (op)
		{
			case '.': push_node(n, create_node(AST_OUT)); break;
			case ',': push_node(n, create_node(AST_IN)); break;
			case '>': push_node(n, create_node(AST_ADDP)); break;
			case '<': push_node(n, create_node(AST_SUBP)); break;
			case '+': push_node(n, create_node(AST_ADD)); break;
			case '-': push_node(n, create_node(AST_SUB)); break;
			case '[':
				block = create_node(AST_BLOCK);
				parse_block(block, block_level + 1);
				push_node(n, block);
				break;
			case ']':
				if (--block_level < 0)
					error("wrong loop placement: missing '['");
				return;
		}
	}

	if (block_level != 0)
		error("wrong loop placement: missing ']'");
}

/* create a abstract syntax tree from all instructions
 * the AST of +>[.-]< looks like this:
 *
 *                BLOCK
 *                |
 *		  OP(INC) - OP(INCP) - BLOCK - OP(DECP)
 *                                     |
 *			               OP(OUT) - OP(DEC)
 */
void parse(void)
{
	debug ("-> start parsing\n");

	// parse recursively all blocks
	ast = create_node(AST_BLOCK);
	parse_block(ast, 0);
}

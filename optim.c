#include "brf.h"

/* remove all nodes between "from" and "to" from the abstract syntax tree.
 * "from" and "to" are exclusive.
 */
static void reduce_nodes(Node *from, Node *to)
{
	Node *tmp;
	Node *deletable = from->next;

	while (deletable != to)
	{
		tmp = deletable;
		deletable = deletable->next;
		free(tmp);
	}

	from->next = to;
}

/* this function works like this:
 *      +++++ => 5+
 *  this can be usesful to reduce the instructions in assembly
 */
static void optimize_reduce_op(Node *op, int type, int new_type)
{
	Node *begin;
	int count = 0;

	while (op)
	{
		if (op->type == type)
		{
			// statement detected
			count++;
			begin = op;
			op = op->next;

			// lets count all following statements of the same type
			while (op && op->type == type)
			{
				count++;
				op = op->next;
			}

			/* reducing is only useful when more than one statement
			was in row */
			if (count > 1)
			{
				begin->type = new_type;
				begin->value = count;
				reduce_nodes(begin, op);
			}

			count = 0;
		}

		// lets search a new statement row
		if (op)
			op = op->next;
	}
}

static void optimize_reducing(Node *n)
{
	optimize_reduce_op(n->child, AST_INC, AST_ADD);
	optimize_reduce_op(n->child, AST_DEC, AST_SUB);
	optimize_reduce_op(n->child, AST_INCP, AST_ADDP);
	optimize_reduce_op(n->child, AST_DECP, AST_SUBP);

	// go through all blocks recursively
	n = n->child;
	while (n)
	{
		if (n->type == AST_BLOCK)
			optimize_reducing(n);

		n = n->next;
	}
}

static void print_ast(Node *n, int level)
{
	int i;

	while (n)
	{
		if (n->type == AST_BLOCK)
		{
			for (i = 0; i < level; i++)
				debug("  ");
			debug("BLOCK\n");
			print_ast(n->child, level + 1);
		}
		else
		{
			for (i = 0; i < level; i++)
				debug("  ");

			char *op = "?";
			debug("OP(");
			switch (n->type)
			{
				case AST_INC: debug("INC"); break;
				case AST_DEC: debug("DEC"); break;
				case AST_INCP: debug("INCP"); break;
				case AST_DECP: debug("DECP"); break;
				case AST_ADD: debug("ADD, %d", n->value); break;
				case AST_SUB: debug("SUB, %d", n->value); break;
				case AST_ADDP: debug("ADDP, %d", n->value); break;
				case AST_SUBP: debug("SUBP, %d", n->value); break;
				case AST_IN: debug("IN"); break;
				case AST_OUT: debug("OUT"); break;
				default:
					debug("???");
			}
			debug(")\n");
		}

		n = n->next;
	}
}

void optimize(void)
{
	debug("-> start optimizing\n");
	optimize_reducing(ast);
	
	// uncomment for printing the optimized ast
	//print_ast(ast->child, 0);
}

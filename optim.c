#include "brf.h"

struct optim_stats_s {
	int reduced_ins;
	int reduced_loops;
} optim_stats;

/* delete all nodes in BLOCK. 'from' is inclusive. */
static void delete_all(Node *from)
{

	while (from)
	{
		Node *tmp = from;
		from = from->next;
		free(tmp);
		optim_stats.reduced_ins++;
	}
}

/* remove all nodes between "from" and "to" from BLOCK.
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
		optim_stats.reduced_ins++;
	}

	from->next = to;
}

/* this function works like this:
 *      +++++ => 5+
 *  this can be usesful to reduce the instructions in assembly
 */
static void optimize_reduce_op(Node *op, int type)
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
				begin->type = type;
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

/* replace [-] and [+] loops with *p = 0 */
static void optimize_zero_loop(Node *op)
{
	if (op->child == NULL)
		return;

	if ((op->child->type == AST_ADD || op->child->type == AST_SUB)
			&& op->child->value == 1
			&& op->child->next == NULL)
	{
		Node *child = op->child;

		// overwrite block with set instruction
		op->type = AST_SET;
		op->op_set.value = 0;
		op->op_set.offset = 0;
		optim_stats.reduced_loops++;

		// delete instructions
		delete_all(child);
	}
}


static int is_cell_op(Node *op)
{
	if (op == NULL)
		return 0;
	int type = op->type;

	return (type == AST_ADD || type == AST_SUB || type == AST_SET);
}

static int is_ptr_op(Node *op)
{
	if (op == NULL)
		return 0;
	int type = op->type;

	return (type == AST_ADDP || type == AST_SUBP);
}

static void optimize_reducing(Node *n)
{
	Node *c = n->child;
	while (c)
	{
		if (c->type == AST_BLOCK)
			optimize_reducing(c);

		c = c->next;
	}

	optimize_reduce_op(n->child, AST_ADD);
	optimize_reduce_op(n->child, AST_SUB);
	optimize_reduce_op(n->child, AST_ADDP);
	optimize_reduce_op(n->child, AST_SUBP);

	optimize_zero_loop(n);
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

			debug("OP(");
			switch (n->type)
			{
				case AST_ADD: debug("ADD, %d", n->value); break;
				case AST_SUB: debug("SUB, %d", n->value); break;
				case AST_ADDP: debug("ADDP, %d", n->value); break;
				case AST_SUBP: debug("SUBP, %d", n->value); break;
				case AST_IN: debug("IN"); break;
				case AST_OUT: debug("OUT"); break;
				case AST_SET: debug("SET, val:%d off: %d", n->op_set.value, n->op_set.offset); break;
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
	debug("->   reduced instructions: %d\n", optim_stats.reduced_ins);
	debug("->   reduced loops: %d\n", optim_stats.reduced_loops);

	// uncomment for printing the optimized ast
	print_ast(ast->child, 0);
}
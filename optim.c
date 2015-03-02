#include "brf.h"

/* remove all nodes between 'from' and 'to' from the abstract syntax list.
 * 'from' and 'to' are exclusive.
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

	if (from)
		from->next = to;
	if (to)
		to->previous = from;
}

/* this function works like this:
 *      +++++ => 5+
 *  this can be usesful to reduce the instructions in assembly
 */
static void optimize_reduce_reps(int type)
{
	Node *begin, *op = asl;
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

void optimize(void)
{
	debug("-> start optimizing\n");
	optimize_reduce_reps(ASL_INC);
	optimize_reduce_reps(ASL_DEC);
	optimize_reduce_reps(ASL_INCP);
	optimize_reduce_reps(ASL_DECP);
}

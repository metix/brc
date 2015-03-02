#include "brf.h"

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

static void optimize_reduce_reps(int type)
{
	Node *begin, *op = asl;
	int count = 0;

	while (op)
	{
		if (op->type == type)
		{
			count++;
			begin = op;
			op = op->next;

			while (op && op->type == type)
			{
				count++;
				op = op->next;
			}

			if (count > 1)
			{
				//printf("optimization detected (reduce_reps_%d): %d\n", type, count);
				begin->value = count;
				reduce_nodes(begin, op);
			}

			count = 0;
		}

		if (op)
			op = op->next;
	}
}

void optimize(void)
{
	optimize_reduce_reps(ASL_INC);
	optimize_reduce_reps(ASL_DEC);
	optimize_reduce_reps(ASL_INCP);
	optimize_reduce_reps(ASL_DECP);
}

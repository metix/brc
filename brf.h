#ifndef _BRF_H_
#define _BRF_H_
#include <stdlib.h>
#include <stdio.h>

#define MAX_CELLS 32768
#define MAX_NESTED_LOOPS 255

extern FILE *fasm;
extern FILE *fin;
extern void error(char *msg);

extern void parse(void);
extern void optimize(void);
extern void generate(void);

/* used in parser and generator */
enum types {
	AST_INC = 1,  /* increment cell-value */
	AST_DEC,  /* decrement cell-value */
	AST_INCP, /* increment cell-pointer */
	AST_DECP, /* decrement cell-pointer */
	AST_OUT,  /* write cell-value in stdout */
	AST_IN,   /* read character from stdin in cell */
	AST_SET,  /* set current cell value */
	AST_ADD,  /* add value to cell */
	AST_SUB,  /* subtract value from cell */
	AST_ADDP, /* add to pointer */
	AST_SUBP, /* subtract from pointer */

	/* virtual types */
	AST_BLOCK
};

/* node used in AST */
typedef struct node_s {
	struct node_s *next;
	struct node_s *parent;
	int type;

	union {
		struct node_s *child;
		int value;
	};
} Node;

/* abstract syntax tree start */
extern Node *ast;

extern char flg_verbose;
#define debug(format, ...) do {                 \
if (flg_verbose)                              \
	fprintf(stderr, format, ##__VA_ARGS__);   \
} while(0)

#endif

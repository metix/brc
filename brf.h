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
	ASL_INC = 1,  /* increment cell-value */
	ASL_DEC,  /* decrement cell-value */
	ASL_INCP, /* increment cell-pointer */
	ASL_DECP, /* decrement cell-pointer */
	ASL_BL,   /* loop-start (bracket left)*/
	ASL_BR,   /* loop-end (bracket right) */
	ASL_OUT,  /* write cell-value in stdout */
	ASL_IN,   /* read character from stdin in cell */
	ASL_SET	  /* set current cell value */
};

/* node used in ASL */
typedef struct node_s {
	struct node_s *next;
	struct node_s *previous;
	int type;
	int value;
} Node;

/* abstract syntax list start */
extern Node *asl;

extern char flg_verbose;
#define debug(format, ...) do {                 \
if (flg_verbose)                              \
	fprintf(stderr, format, ##__VA_ARGS__);   \
} while(0)

#endif

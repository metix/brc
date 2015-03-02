/* a small brainfuck-compiler for learning purposes
 * cells-width: 8-bit
 * cells-count: MAX_CELLS
 * max-nested-loops: MAX_NESTED_LOOPS
 *
 * -o (output file)
 * -S (output as assembler-file .s)
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>
#include <libgen.h>
#include "brf.h"

char *outfile, *infile, *objfile, *asmfile;
FILE *fin, *fasm;

char flg_asm, flg_compile, flg_optim, flg_verbose;

static void usage()
{
	fprintf(stderr,
		"usage: ./brc [options] [-o <output-file>] <source-file>\n"
		"options:\n"
		"  -S 		do not call assembler (output as .s)\n"
		"  -c 		do not call linker (output as .o)\n"
		"  -O		enable optimization\n"
		"  -v		enable debug-messages\n"
		"  -h		print this screen"
		"\n\n");

	exit(1);
}

void error(char *msg)
{
	fprintf(stderr, "error: %s\n", msg);
	exit(1);
}

static char *suffix_replace(char *f, char suffix)
{
	static char buf[255];
	strcpy(buf, f);
	char *p = buf + strlen(buf) - 1;
	*p = suffix;
	return buf;
}

static void parseopt(int argc, char **argv)
{
	int c;
	while ((c = getopt(argc, argv, "vSOcho:")) != -1)
	{
		switch (c)
		{
			case 'o':
				outfile = optarg;
				break;
			case 'S':
				flg_asm = 1;
				/* fall through */
			case 'c':
				flg_compile = 1;
				break;
			case 'O':
				flg_optim = 1;
				break;
			case 'v':
				flg_verbose = 1;
				break;
			case 'h':
			default:
				usage();
		}
	}

	if (optind != argc - 1)
		usage();

	infile = argv[optind];
}

static void open_files(void)
{
	/* when no outputfile is given */
	if (outfile == NULL)
	{
		if (flg_asm)
			outfile = suffix_replace(infile, 's');
		else if (flg_compile)
			outfile = suffix_replace(infile, 'o');
		else
			outfile = "a.out";
	}

	if (flg_asm)
		asmfile = outfile;
	else
		asmfile = "/tmp/brc_tmp_asm.s";

	if (flg_compile)
		objfile = outfile;
	else
		objfile = "/tmp/brc_tmp_obj.o";

	fin = fopen(infile, "r");

	if (!fin)
	{
		perror("fopen");
		error("couldn't open input file");
	}

	fasm = fopen(asmfile, "w");

	if (!fasm)
	{
		perror("fopen");
		error("couldn't write asm file");
	}

	debug("outfile: %s\ninfile: %s\nasmfile: %s\nobjfile: %s\n",
		outfile, infile, asmfile, objfile);
}

static void close_files(void)
{
	fclose (fin);
	fclose (fasm);
}

static void call_assembler(void)
{
	pid_t pid = fork();

	if (pid < 0) perror("fork");
	if (pid == 0)
	{
		execlp("as", "as", "-c", "-o", objfile, asmfile, (char *)NULL);
		error("execl failed");
	}

	int status;
	waitpid(pid, &status, 0);
	if (status < 0)
		error("as failed");
}

static void call_linker(void)
{
	pid_t pid = fork();

	if (pid < 0) perror("fork");
	if (pid == 0)
	{
		execlp("ld", "ld", "-o", outfile, objfile, (char *)NULL);
		error("execl failed");
	}

	int status;
	waitpid(pid, &status, 0);
	if (status < 0)
		error("ld failed");
}

int main(int argc, char **argv)
{
	parseopt(argc, argv);
	open_files();

	parse();
	if (flg_optim)
		optimize();
	generate();

	close_files();

	if (!flg_asm)
		call_assembler();
	if (!flg_compile)
		call_linker();

	return 0;
}

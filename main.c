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

char *outfile, *infile;
FILE *fin, *fasm;

char flg_asm;
char flg_compile;

extern void generate(void);

static void usage()
{
	fprintf(stderr,
		"usage: ./brc <source-file> -o <output-file>\n"
		"  -S 		do not call assembler (output as .s)\n"
		"  -c 		do not call linker (output as .o)"
		"\n\n");

	exit(1);
}

static void error(char *msg)
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
	while ((c = getopt(argc, argv, "o:S:c:h")) != -1)
	{
		switch (c)
		{
			case 'o':
				outfile = optarg;
				break;
			case 'S':
				flg_asm = 1;
				break;
			case 'c':
				flg_compile = 1;
				break;
			case 'h':
			default:
				usage();
		}
	}

	if (optind != argc - 1)
		usage();

	infile = argv[optind];

	if (outfile == NULL)
	{
		if (!flg_asm)
			outfile = suffix_replace(infile, 'o');
		else
			outfile = suffix_replace(infile, 's');
	}
}

int main(int argc, char **argv)
{
	parseopt(argc, argv);

	fin = fopen(infile, "r");

	if (!fin)
	{
		perror("fopen");
		error("couldn't open input file");
	}

	char *asmfile;
	if (!flg_asm)
		asmfile = "/tmp/brc_tmp_asm.s";
	else
		asmfile = outfile;

	fasm = fopen(asmfile, "w");

	if (!fasm)
	{
		perror("fopen");
		error("couldn't open asm file");
	}

	generate();

	fclose (fin);
	fclose (fasm);

	if (!flg_asm)
	{
		pid_t pid = fork();

		if (pid < 0) perror("fork");
		if (pid == 0)
		{
			execlp("as", "as", "-c", "-o", outfile, asmfile, (char *)NULL);
			error("execl failed");
		}

		int status;
		waitpid(pid, &status, 0);
		if (status < 0)
			error("as failed");
	}

	if (!flg_compile)
	{
		pid_t pid = fork();

		if (pid < 0) perror("fork");
		if (pid == 0)
		{
			execlp("ld", "ld", "-o", "a.out", suffix_replace(outfile, 'o'), (char *)NULL);
			error("execl failed");
		}

		int status;
		waitpid(pid, &status, 0);
		if (status < 0)
			error("ld failed");
	}


	return 0;
}

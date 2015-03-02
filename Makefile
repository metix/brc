CC = gcc
CFLAGS = -Wall -Werror

TARGET = brc

SRCS = main.c gen.c

all:
	$(CC) $(CFLAGS) $(SRCS) -o $(TARGET)

test: all
	./brc test.b
	./brc -S -o test.b
	./a.out | hexdump -C

clean:
	rm -rf *.o
	rm -rf $(TARGET)
	rm -rf *.s
	rm -rf core
	rm -rf *.out

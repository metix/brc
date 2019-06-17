CC = gcc
CFLAGS = -Wall -g

TARGET = brc

SRCS = main.c gen.c parser.c optim.c

all:
	$(CC) $(CFLAGS) $(SRCS) -o $(TARGET)

test: all
	./brc test.b
	./brc -SOv test.b
	./a.out | hexdump -C

clean:
	rm -rf *.o
	rm -rf $(TARGET)
	rm -rf *.s
	rm -rf core
	rm -rf *.out

FLAGS = -std=gnu11 -O2 $(CFLAGS)
CC = gcc
OBJS = builtins.o env.o mem.o pred.o types.o

all: skeem

skeem: $(OBJS) skeem.o
	$(CC) $(FLAGS) $(OBJS) skeem.o -o skeem

tests: $(OBJS) tests.c
	$(CC) $(FLAGS) $(OBJS) tests.c -o tests

skeem.o: skeem.c
	$(CC) $(FLAGS) -c skeem.c

builtins.o: builtins.c
	$(CC) $(FLAGS) -c builtins.c

env.o: env.c
	$(CC) $(FLAGS) -c env.c

mem.o: mem.c
	$(CC) $(FLAGS) -c mem.c

pred.o: pred.c
	$(CC) $(FLAGS) -c pred.c

types.o: types.c
	$(CC) $(FLAGS) -c types.c

clean:
	rm *.o

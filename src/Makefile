FLAGS = -std=gnu11 $(BUILDFLAGS) $(CFLAGS)
CCOMP = $(if $(CC), $(CC), cc)
OBJS = builtins.o mem.o types.o token.o

all: skeem

skeem: $(OBJS) skeem.o
	$(CCOMP) $(FLAGS) $(OBJS) skeem.o -o skeem

tests: $(OBJS) tests.c
	$(CCOMP) $(FLAGS) $(OBJS) tests.c -o tests

skeem.o: skeem.c
	$(CCOMP) $(FLAGS) -c skeem.c

builtins.o: builtins.c
	$(CCOMP) $(FLAGS) -c builtins.c

mem.o: mem.c
	$(CCOMP) $(FLAGS) -c mem.c

types.o: types.c
	$(CCOMP) $(FLAGS) -c types.c

token.o: token.c
	$(CCOMP) $(FLAGS) -c token.c

clean:
	rm *.o
	rm skeem

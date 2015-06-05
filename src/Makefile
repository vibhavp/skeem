NAME = skeem
EXENAME = skeem
SRCS = builtins.c mem.c token.c types.c
OBJS = builtins.o mem.o token.o types.o
DOBJS = builtins.do mem.do token.do types.do
FLAGS = -std=gnu1x $(CFLAGS)
DEBUGFLAGS = -g -DDEBUG $(FLAGS)
RELEASEFLAGS = -O2 $(FLAGS)

default: release

%.o: %.c
	$(CC) $(RELEASEFLAGS) -c $< -o $@
%.do: %.c
	$(CC) $(DEBUGFLAGS) -c $< -o $@

skeem.o: builtins.c mem.c token.c types.c
skeem.do: builtins.c mem.c token.c types.c

debug: $(DOBJS) skeem.do
	$(CC) $(DEBUGFLAGS) $(DOBJS) skeem.do -o skeem

release: $(OBJS) skeem.o
	$(CC) $(RELEASEFLAGS) $(OBJS) skeem.o -o skeem

clean:
	rm -f *.o *.do
	rm -f skeem

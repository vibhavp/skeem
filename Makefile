all: release

debug:
	+$(MAKE) -C src CFLAGS="-g -DDEBUG"

release:
	+$(MAKE) -C src CFLAGS='-O2'

clean:
	cd src; make clean

all: release

debug:
	+$(MAKE) -C src CFLAGS='-g'

release:
	+$(MAKE) -C src

clean:
	cd src; make clean

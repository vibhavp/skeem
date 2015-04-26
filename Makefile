all: release

debug:
	+$(MAKE) -C src BUILDFLAGS="-g -DDEBUG"

release:
	+$(MAKE) -C src BUILDFLAGS='-O2'

clean:
	cd src; make clean

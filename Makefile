all: release

debug:
	+$(MAKE) -C src BUILDFLAGS="-g -DDEBUG"

release:
	+$(MAKE) -C src BUILDFLAGS='-O2 -fweb -fno-trapping-math -fno-signaling-nans'

clean:
	cd src; make clean

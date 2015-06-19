all: release

debug:
	cd src; make debug

release:
	cd src; make

tests:
	cd src; make tests

clean:
	cd src; make clean
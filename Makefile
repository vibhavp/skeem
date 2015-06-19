all: release

debug:
	cd src; make debug

release:
	cd src; make release

tests:
	cd src; make tests

clean:
	cd src; make clean
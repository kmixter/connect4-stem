all:
	make -C src
	make -C src -f Makefile.arduino

clean:
	make -C src clean
	make -C src -f Makefile.arduino clean

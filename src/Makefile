#
# ABACUS
# ------------------------
# by Gokul Soundararajan
#
# Makefile
#

include config.mk

all: clean libs abacus libabacus.a

libabacus.a: libs abacus
	ar rcs libabacus.a lib/*.o ab/*.o

libs:
	cd lib; make
abacus:
	cd ab; make

clean:
	rm -rf *.o *.exe *.a *.so *.*~ *~
	cd include; make clean
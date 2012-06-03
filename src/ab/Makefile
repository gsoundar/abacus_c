#
# ABACUS
# ------------------------
# by Gokul Soundararajan
#
# Makefile
#

include ../config.mk

all: clean libs
libs: abacus.o

abacus.o: abacus.c
	${CC} -c ${CFLAGS} ${INCLUDES} abacus.c -o abacus.o

clean:
	rm -rf *.o *.exe *.a *.so *.*~ *~

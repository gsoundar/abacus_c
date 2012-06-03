#
# ABACUS
# ------------------------
# by Gokul Soundararajan
#
# Makefile
#

include ../config.mk

all: clean libs
libs: hashtable.o md5.o guid.o

hashtable.o: hashtable.c
	${CC} -c ${CFLAGS} ${INCLUDES} hashtable.c -o hashtable.o
md5.o: md5.c
	${CC} -c ${CFLAGS} ${INCLUDES} md5.c -o md5.o
guid.o: guid.c
	${CC} -c ${CFLAGS} ${INCLUDES} guid.c -o guid.o

clean:
	rm -rf *.o *.exe *.a *.so *.*~ *~

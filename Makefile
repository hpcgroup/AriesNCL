# If make fails, make sure papi module is loaded! Also try unloading darshan

CC	= cc
CFLAGS	= -O3 -g -Wall
LFLAGS	= -ldl

all: mpitest

mpitest: test.cpp libariesncl.a
	${CC} ${CFLAGS} -o mpitest test.cpp -L./libariesncl.a ${LFLAGS}

libariesncl.a: AriesCounters.h
	${CC} ${CFLAGS} -c AriesCounters.h ${LFLAGS}
	ar -cvq libariesncl.a AriesCounters.o

clean : 
	rm -f mpitest AriesCounters.o libariesncl.a
	

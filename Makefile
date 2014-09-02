# If make fails, make sure papi module is loaded! Also try unloading darshan

CC = cc
CFLAGS = -O3 -g -Wall
LFLAGS = -ldl
all: mpitest.out

mpitest.out: test3.cpp libariescounters.a
	${CC} ${CFLAGS} -o mpitest.out test3.cpp -L./libariescounters.a ${LFLAGS}

libariescounters.a: AriesCounters.h
	${CC} ${CFLAGS} -c AriesCounters.h ${LFLAGS}
	ar -cvq libariescounters.a AriesCounters.o

clean : 
	rm -f mpitest.out AriesCounters.o libariescounters.a
	

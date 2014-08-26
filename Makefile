# If make fails, make sure papi module is loaded! Also try unloading darshan

CFLAGS = -O2 -g -Wall
LFLAGS = -lstdc++
all: counter.out sample.out

counter.out : test.c libariescounters.a
	cc ${CFLAGS} -o counter.out test.c -L./libariescounters.a

sample.out: test2.cpp libariescounters.a
	cc ${CFLAGS} -o sample.out test2.cpp -L./libariescounters.a

mpitest.out: test3.cpp libariescounters.a
	cc ${CFLAGS} -o mpitest.out test3.cpp -L./libariescounters.a

#libariescounters.a: AriesCounters.hpp AriesCounters.cpp
#	CC ${CFLAGS} -c AriesCounters.cpp
#	ar -cvq libariescounters.a AriesCounters.o

libariescounters.a: AriesCounters.h
	cc ${CFLAGS} -c AriesCounters.h
	ar -cvq libariescounters.a AriesCounters.o

clean : 
	rm -f counter.out sample.out mpitest.out AriesCounters.o libariescounters.a
	

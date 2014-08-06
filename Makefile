# If make fails, make sure papi module is loaded!

CFLAGS = -O0 -g -Wall
LFLAGS = -lstdc++
all: counter.out solo.out

counter.out : test.c libariescounters.a
	cc ${CFLAGS} -o counter.out test.c -L./libariescounters.a

solo.out: test2.cpp libariescounters.a
	cc ${CFLAGS} -o solo.out test2.cpp -L./libariescounters.a

#libariescounters.a: AriesCounters.hpp AriesCounters.cpp
#	CC ${CFLAGS} -c AriesCounters.cpp
#	ar -cvq libariescounters.a AriesCounters.o

libariescounters.a: AriesCounters.h
	cc ${CFLAGS} -c AriesCounters.h
	ar -cvq libariescounters.a AriesCounters.o

clean : 
	rm -f counter.out solo.out AriesCounters.o libariescounters.a
	

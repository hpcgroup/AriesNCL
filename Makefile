##############################################################################
# Copyright (c) 2014, Lawrence Livermore National Security, LLC.
# Produced at the Lawrence Livermore National Laboratory.
#
# Written by:
#     Dylan Wang <dylan.wang@gmail.com>
#     Staci Smith <smiths949@email.arizona.edu>
#     Abhinav Bhatele <bhatele@llnl.gov>
#
# LLNL-CODE-678960. All rights reserved.
#
# This file is part of AriesNCL. For details, see:
# https://github.com/LLNL/ariesncl
# Please also read the LICENSE file for our notice and the LGPL.
##############################################################################

# If make fails, make sure papi module is loaded! Also try unloading darshan
CC	= cc
CFLAGS	= -O3 -g -Wall
LFLAGS	= -ldl

all: mpitest mpitest2 mpitest3

mpitest: test.c libariesncl.a
	${CC} ${CFLAGS} -o mpitest test.c -L./libariesncl.a ${LFLAGS}

mpitest2: test2.c libariesncl.a
	${CC} ${CFLAGS} -o mpitest2 test2.c -L./libariesncl.a ${LFLAGS}
	
mpitest3: test3.c libariesncl.a
	${CC} ${CFLAGS} -o mpitest3 test3.c -L./libariesncl.a ${LFLAGS}
	
libariesncl.a: AriesCounters.h
	${CC} ${CFLAGS} -c AriesCounters.h ${LFLAGS}
	ar -cvq libariesncl.a AriesCounters.o

clean: 
	rm -f mpitest* AriesCounters.o libariesncl.a

reallyclean:
	rm -f mpitest* AriesCounters.o libariesncl.a *.yaml	

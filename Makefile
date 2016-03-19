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

all: mpitest RecoverAriesCounters 

mpitest: test.cpp librariesncl.a
	${CC} ${CFLAGS} -o mpitest test.cpp -L./librariesncl.a ${LFLAGS}

librariesncl.a: AriesCounters.h
	${CC} ${CFLAGS} -c AriesCounters.h ${LFLAGS}
	ar -cvq librariesncl.a AriesCounters.o

RecoverAriesCounters: RecoverAriesCounters.c librariesncl.a
	${CC} ${CFLAGS} -o RecoverAriesCounters RecoverAriesCounters.c -L./librariesncl.a ${LFLAGS}

clean : 
	rm -f mpitest RecoverAriesCounters AriesCounters.o librariesncl.a
	

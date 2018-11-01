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

all: libariesncl.a

libariesncl.a: AriesCounters.c
	${CC} ${CFLAGS} -c AriesCounters.c
	ar -cvq libariesncl.a AriesCounters.o

clean: 
	rm -f AriesCounters.o libariesncl.a

reallyclean:
	rm -f AriesCounters.o libariesncl.a *.yaml	

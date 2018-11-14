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

CC	= cc
CFLAGS	= -O3 -g -Wall

all: regions

regions: regions.c
	${CC} ${CFLAGS} regions.c -o regions -I../src -L../src -lariesncl

clean: 
	rm -f regions

reallyclean:
	rm -f regions *.yaml *.bin

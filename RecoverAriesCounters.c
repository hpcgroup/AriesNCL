//////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2014, Lawrence Livermore National Security, LLC.
// Produced at the Lawrence Livermore National Laboratory.
//
// Written by:
//     Dylan Wang <dylan.wang@gmail.com>
//     Staci Smith <smiths949@email.arizona.edu>
//     Abhinav Bhatele <bhatele@llnl.gov>
//
// LLNL-CODE-678960. All rights reserved.
//
// This file is part of AriesNCL. For details, see:
// https://github.com/LLNL/ariesncl
// Please also read the LICENSE file for our notice and the LGPL.
//////////////////////////////////////////////////////////////////////////////


#include <stdio.h>
#include "AriesCounters.h"

int main( int argc, char** argv ) {
  if (argc < 4) {
    printf("Usage: %s <binary counters file> <reporting rank mod> <num reporting ranks>\n", argv[0]);
    return 1;
  }
  char* filename = argv[1];
  int reporting_ranks_mod = atoi(argv[2]);
  int reporting_ranks = atoi(argv[3]);

  char** AC_events;
  int AC_event_count;
  ReadAriesCountersFile(&AC_events, &AC_event_count);

  ReadAriesCountersBinary(reporting_ranks, reporting_ranks_mod, filename, &AC_events, &AC_event_count);

  return 0;
}

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

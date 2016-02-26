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

#ifndef ARIESCOUNTERS_H
#define ARIESCOUNTERS_H

#include <papi.h>
#include <stdio.h> // I/O
#include <stdlib.h> // Malloc, atoi
#include <string.h> // strcpy, strcat
#include <unistd.h> // sleep
#include <ctype.h> // isdigit
#include <sys/time.h> // gettimeofday

#include "mpi.h"

#define MAX_COUNTER_NAME_LENGTH 70

/* counter for regions/timesteps profiled */
int region = 1;

/* utilities for timing each region/timestep */
struct timeval tempo1, tempo2;
long elapsed_utime; 	/* elapsed time in microseconds */
long elapsed_mtime; 	/* elapsed time in milliseconds */
long elapsed_seconds; 	/* diff between seconds counter */
long elapsed_useconds; 	/* diff between microseconds counter */

void StartSysTimer() {
  gettimeofday(&tempo1, NULL);
}

void EndSysTimer() {
  gettimeofday(&tempo2, NULL);

  elapsed_seconds = tempo2.tv_sec - tempo1.tv_sec;
  elapsed_useconds = tempo2.tv_usec - tempo1.tv_usec;

  elapsed_utime = (elapsed_seconds) * 1000000 + elapsed_useconds;
  elapsed_mtime = ((elapsed_seconds) * 1000 + elapsed_useconds/1000.0) + 0.5;
}

/* library methods */
void InitAriesCounters(int my_rank, int reporting_rank_mod, int* AC_event_set, char*** AC_events, long long** AC_values, int* AC_event_count)
{
  if (my_rank % reporting_rank_mod != 0)
  {
    *AC_event_set = 0;
    *AC_events = 0;
    *AC_values = 0;
    *AC_event_count = 0;
    return;
  }

  *AC_event_set = PAPI_NULL;

  FILE* fp;

  fp = fopen("counters.txt", "r");
  if (!fp)
  {
    printf("counters.txt file not found\n");
  }

  // Get number of counters so we can malloc space
  // we are relying on \n, so it will cause errors later if the file pointer does not end in a blank line.
  int linecount=0;
  while(!feof(fp))
  {
    char ch = fgetc(fp);
    if(ch == '\n')
    {
      linecount++;
    }
  }	
  *AC_event_count = linecount;

  // Make space for all the counters
  *AC_events = (char**)malloc(sizeof(char*) * (*AC_event_count));
  // we could malloc a size perfectly fit for each counter,
  // but the space difference is so minimal
  int i;
  for (i = 0; i < *AC_event_count; i++)
  {
    (*AC_events)[i] = (char*)malloc(sizeof(char) * MAX_COUNTER_NAME_LENGTH);
  }

  // since we read one pass for line count, we need to reset
  fseek(fp, 0, SEEK_SET);

  char line[MAX_COUNTER_NAME_LENGTH];
  i = 0; // repurpose this variable to keep track of AC_events index.

  while (fgets(line, MAX_COUNTER_NAME_LENGTH, fp) != NULL)
  {
    // fgets will leave the newline. so remove it
    size_t ln = strlen(line) - 1;
    if (line[ln] == '\n')
      line[ln] = '\0';

    sscanf("%s", line);
    strcpy((*AC_events)[i], line);

    i++;
  }
  fclose(fp);

  *AC_values = (long long *)malloc(sizeof(long long) * (*AC_event_count));

  // Initialize PAPI
  PAPI_library_init(PAPI_VER_CURRENT);
  PAPI_create_eventset(AC_event_set);
  int code = 0;
  for (i = 0; i < *AC_event_count; i++)
  {
    PAPI_event_name_to_code((*AC_events)[i], &code);
    PAPI_add_event(*AC_event_set, code);
  }
}

void StartRecordAriesCounters(int my_rank, int reporting_rank_mod, int* AC_event_set, char*** AC_events, long long** AC_values, int* AC_event_count)
{
  if (my_rank % reporting_rank_mod != 0) { return; }

  if (my_rank == 0) {	
    printf("counters: Starting counters for timestep.\n");
  }

  PAPI_start(*AC_event_set);

  // Start a timer to measure elapsed time
  StartSysTimer();	
}

// This is the value to put in the output file with format counterData-XXX.txt
void EndRecordAriesCounters(MPI_Comm* mod16_comm, int my_rank, int reporting_rank_mod, double run_time, int* AC_event_set, char*** AC_events, long long** AC_values, int* AC_event_count)
{
  if (my_rank % reporting_rank_mod != 0) { return; }

  if (my_rank == 0) { 
    printf("counters: Writing out counters for timestep.\n");
  }

  // Stop timer for elapsed time
  EndSysTimer();

  int number_of_reporting_ranks;
  // Array to store counter data
  long long * counter_data;

  PAPI_stop(*AC_event_set, *AC_values);
  PAPI_reset(*AC_event_set);

  int size;
  MPI_Comm_size(MPI_COMM_WORLD, &size);
  MPI_Comm_rank (MPI_COMM_WORLD, &my_rank);
  number_of_reporting_ranks = (size-1)/reporting_rank_mod + 1; // integer division is fine; we want the floor

  /* Rank 0 needs to write out counter info.
     First space needs to be allocated to recieve the data */
  // Array of longlong equal to number of reporting ranks * number of counters
  if (my_rank == 0)
  {
    counter_data = (long long*)malloc(sizeof(long long) * number_of_reporting_ranks * *AC_event_count); 
  }

  /* MPI_Gather to collect counters from all ranks % 0 */
  MPI_Gather(*AC_values, *AC_event_count, MPI_LONG_LONG,
      counter_data, *AC_event_count, MPI_LONG_LONG, 0, *mod16_comm);

  if (my_rank == 0)
  {
    int i,j;

    char filename[30];
    sprintf(filename, "networktiles.%d.yaml", region);
    FILE* fp = fopen(filename, "w");
    /* print out in yaml -- same format as in boxfish */
    fprintf(fp, "---\nkey: ARIESCOUNTER_NETWORK\n---\n");
    fprintf(fp, "- jobid: %s\n", getenv("PBS_JOBID"));
    fprintf(fp, "- timestamp: %s\n", getenv("savedir"));
    fprintf(fp, "- elapsedtime: %ld\n---\n", elapsed_mtime);
    fprintf(fp, "- [mpirank, int32]\n");
    fprintf(fp, "- [tilex, int32]\n");
    fprintf(fp, "- [tiley, int32]\n");

    //for (i=0; i<*AC_event_count; i++)
    //{
    //	fprintf(fp, "- [\"%s\", int128]\n", (*AC_events)[i]);
    //}
    fprintf(fp, "- [\"COLBUF_PERF_STALL_RQ:COL_BUF_PERF_STALL_RQ\", int128]\n");
    fprintf(fp, "- [\"COLBUF_PERF_STALL_RQ:VC_PTR\", int128]\n");
    fprintf(fp, "- [\"COLBUF_PERF_STALL_RS:COL_BUF_PERF_STALL_RS\", int128]\n");
    fprintf(fp, "- [\"COLBUF_PERF_STALL_RS:VC_PTR\", int128]\n");
    fprintf(fp, "- [\"INQ_PRF_INCOMING_FLIT_VC0\", int128]\n");
    fprintf(fp, "- [\"INQ_PRF_INCOMING_FLIT_VC1\", int128]\n");
    fprintf(fp, "- [\"INQ_PRF_INCOMING_FLIT_VC2\", int128]\n");
    fprintf(fp, "- [\"INQ_PRF_INCOMING_FLIT_VC3\", int128]\n");
    fprintf(fp, "- [\"INQ_PRF_INCOMING_FLIT_VC4\", int128]\n");
    fprintf(fp, "- [\"INQ_PRF_INCOMING_FLIT_VC5\", int128]\n");
    fprintf(fp, "- [\"INQ_PRF_INCOMING_FLIT_VC6\", int128]\n");
    fprintf(fp, "- [\"INQ_PRF_INCOMING_FLIT_VC7\", int128]\n");
    fprintf(fp, "- [\"INQ_PRF_INCOMING_PKT_VC0_FILTER_FLIT0_CNT\", int128]\n");
    fprintf(fp, "- [\"INQ_PRF_INCOMING_PKT_VC1_FILTER_FLIT1_CNT\", int128]\n");
    fprintf(fp, "- [\"INQ_PRF_INCOMING_PKT_VC2_FILTER_FLIT2_CNT\", int128]\n");
    fprintf(fp, "- [\"INQ_PRF_INCOMING_PKT_VC3_FILTER_FLIT3_CNT\", int128]\n");
    fprintf(fp, "- [\"INQ_PRF_INCOMING_PKT_VC4_FILTER_FLIT4_CNT\", int128]\n");
    fprintf(fp, "- [\"INQ_PRF_INCOMING_PKT_VC5_FILTER_FLIT5_CNT\", int128]\n");
    fprintf(fp, "- [\"INQ_PRF_INCOMING_PKT_VC6_FILTER_FLIT6_CNT\", int128]\n");
    fprintf(fp, "- [\"INQ_PRF_INCOMING_PKT_VC7_FILTER_FLIT7_CNT\", int128]\n");
    fprintf(fp, "- [\"INQ_PRF_MATCH_FLIT_3_TO_0_FILTERING_CNT\", int128]\n");
    fprintf(fp, "- [\"INQ_PRF_MATCH_FLIT_7_TO_4_FILTERING_CNT\", int128]\n");
    fprintf(fp, "- [\"INQ_PRF_PKT_TO_DEAD_LINK_CNT\", int128]\n");
    fprintf(fp, "- [\"INQ_PRF_ROWBUS_2X_USAGE_CNT\", int128]\n");
    fprintf(fp, "- [\"INQ_PRF_ROWBUS_STALL_CNT\", int128]\n");

    fprintf(fp, "...");

    // for each reporting rank...
    for (i=0; i<number_of_reporting_ranks; i++)
    {
      int reporting_rank = i * reporting_rank_mod;
      int new_coord = 1; // 1=True, 0=False. This determines whether we should go to new line
      int x=-1, y=-1;

      // loop through counters of this particular rank
      // this loop assumes that once a coordinate comes up, (i.e. a particular XY)
      // all the other counters of that coord will directly follow
      // and the coordinate will not show up again once a different coordinate appears
      for (j=0; j<*AC_event_count; j++)
      {
	// need to extract network tile coordinates from counter name
	// XY is either at pos 7 and 9 or 10 and 12
	if (isdigit((*AC_events)[j][7]))
	{
	  if ((*AC_events)[j][7] - '0' != x || (*AC_events)[j][9] - '0' != y)
	  {
	    new_coord = 1;
	  }
	  x = (*AC_events)[j][7] - '0';
	  y = (*AC_events)[j][9] - '0';
	}
	else
	{
	  if ((*AC_events)[j][10] - '0' != x || (*AC_events)[j][12] - '0' != y)
	  {
	    new_coord = 1;
	  }
	  x = (*AC_events)[j][10] - '0';
	  y = (*AC_events)[j][12] - '0';
	}
	if (new_coord && x != 5)
	{
	  fprintf(fp, "\n%d %d %d", reporting_rank, x, y);
	  new_coord = 0;
	}
	if (x != -1 && x != 5 && y != -1)
	{
	  fprintf(fp, " %lld", counter_data[i * (*AC_event_count) + j]);
	}
      }
    }
    fclose(fp);

    sprintf(filename, "proctiles.%d.yaml", region);
    fp = fopen(filename, "w");
    /* print out in yaml -- same format as in boxfish */
    fprintf(fp, "---\nkey: ARIESCOUNTER_PROC\n---\n");
    fprintf(fp, "- jobid: %s\n", getenv("PBS_JOBID"));
    fprintf(fp, "- timestamp: %s\n", getenv("savedir"));
    fprintf(fp, "- elapsedtime: %ld\n---\n", elapsed_mtime);
    fprintf(fp, "- [mpirank, int32]\n");
    fprintf(fp, "- [tilex, int32]\n");
    fprintf(fp, "- [tiley, int32]\n");

    fprintf(fp, "- [\"COLBUF_PERF_STALL_RQ\", int128]\n");
    fprintf(fp, "- [\"COLBUF_PERF_STALL_RS\", int128]\n");
    fprintf(fp, "- [\"INQ_PRF_INCOMING_FLIT_VC0\", int128]\n");
    fprintf(fp, "- [\"INQ_PRF_INCOMING_FLIT_VC4\", int128]\n");
    fprintf(fp, "- [\"INQ_PRF_INCOMING_PKT_VC0_FILTER_FLIT0_CNT\", int128]\n");
    fprintf(fp, "- [\"INQ_PRF_INCOMING_PKT_VC1_FILTER_FLIT1_CNT\", int128]\n");
    fprintf(fp, "- [\"INQ_PRF_INCOMING_PKT_VC2_FILTER_FLIT2_CNT\", int128]\n");
    fprintf(fp, "- [\"INQ_PRF_INCOMING_PKT_VC3_FILTER_FLIT3_CNT\", int128]\n");
    fprintf(fp, "- [\"INQ_PRF_INCOMING_PKT_VC4_FILTER_FLIT4_CNT\", int128]\n");
    fprintf(fp, "- [\"INQ_PRF_INCOMING_PKT_VC5_FILTER_FLIT5_CNT\", int128]\n");
    fprintf(fp, "- [\"INQ_PRF_INCOMING_PKT_VC6_FILTER_FLIT6_CNT\", int128]\n");
    fprintf(fp, "- [\"INQ_PRF_INCOMING_PKT_VC7_FILTER_FLIT7_CNT\", int128]\n");
    fprintf(fp, "- [\"INQ_PRF_MATCH_FLIT_3_TO_0_FILTERING_CNT\", int128]\n");
    fprintf(fp, "- [\"INQ_PRF_MATCH_FLIT_7_TO_4_FILTERING_CNT\", int128]\n");
    fprintf(fp, "- [\"INQ_PRF_PKT_TO_DEAD_LINK_CNT\", int128]\n");
    fprintf(fp, "- [\"INQ_PRF_REQ_ROWBUS_STALL_CNT\", int128]\n");
    fprintf(fp, "- [\"INQ_PRF_ROWBUS_2X_USAGE_CNT\", int128]\n");
    fprintf(fp, "- [\"INQ_PRF_RSP_ROWBUS_STALL_CNT\", int128]\n");

    fprintf(fp, "...");

    // for each reporting rank...
    for (i=0; i<number_of_reporting_ranks; i++)
    {
      int reporting_rank = i * reporting_rank_mod;
      int new_coord = 1; // 1=True, 0=False. This determines whether we should go to new line
      int x=-1, y=-1;

      // loop through counters of this particular rank
      // this loop assumes that once a coordinate comes up, (i.e. a particular XY)
      // all the other counters of that coord will directly follow
      // and the coordinate will not show up again once a different coordinate appears
      for (j=0; j<*AC_event_count; j++)
      {
	// need to extract network tile coordinates from counter name
	// XY is either at pos 7 and 9 or 10 and 12
	if (isdigit((*AC_events)[j][7]))
	{
	  if ((*AC_events)[j][7] - '0' != x || (*AC_events)[j][9] - '0' != y)
	  {
	    new_coord = 1;
	  }
	  x = (*AC_events)[j][7] - '0';
	  y = (*AC_events)[j][9] - '0';
	}
	else
	{
	  if ((*AC_events)[j][10] - '0' != x || (*AC_events)[j][12] - '0' != y)
	  {
	    new_coord = 1;
	  }
	  x = (*AC_events)[j][10] - '0';
	  y = (*AC_events)[j][12] - '0';
	}
	if (new_coord && x == 5 && y != -1)
	{
	  fprintf(fp, "\n%d %d %d", reporting_rank, x, y);
	  new_coord = 0;
	}
	if (x == 5 && y != -1)
	{
	  fprintf(fp, " %lld", counter_data[i * (*AC_event_count) + j]);
	}
      }
    }
    fclose(fp);

    region++;

    // Cleanup counter_data
    // If this is called really often, it may be better to malloc once
    // and save it for future uses.
    free(counter_data);
  }
}

void FinalizeAriesCounters(int my_rank, int reporting_rank_mod, int* AC_event_set, char*** AC_events, long long** AC_values, int* AC_event_count)
{
  if (my_rank % reporting_rank_mod != 0) { return; }

  // cleanup malloc
  int i;
  for (i = 0; i < *AC_event_count; i++)
  {
    free((*AC_events)[i]);
  }
  free(*AC_events);
  free(*AC_values);

  PAPI_cleanup_eventset(*AC_event_set);
  PAPI_destroy_eventset(AC_event_set);
  PAPI_shutdown();
}

#endif

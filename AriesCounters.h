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

#include "mpi.h"


/* Function for dumping a timestamp. */
void timestamp();

#define MAX_COUNTER_NAME_LENGTH 70

void StartSysTimer();
void EndSysTimer();

/* Store counters in memory in a linked list, reporting them to rank 0 at the end. */
struct timestep_counters {
	long long *counters;
	int timestep;
	struct timestep_counters *next;
};

/* library methods */
void ReadAriesCountersFile(char*** AC_events, int* AC_event_count);
void InitAriesCounters(char *progname, int my_rank, int reporting_rank_mod, int* AC_event_set, char*** AC_events, long long** AC_values, int* AC_event_count);

/* Start recording Aries counters with timestamp printed (useful for timing in coarse-grain profiling) */
void StartRecordAriesCounters(int my_rank, int reporting_rank_mod, int* AC_event_set, char*** AC_events, long long** AC_values, int* AC_event_count);

/* Start recording Aries counters WITHOUT timestamp printed (useful for fine-grain profiling, where too many print statements would be overwhelming) */
void StartRecordQuietAriesCounters(int my_rank, int reporting_rank_mod, int* AC_event_set, char*** AC_events, long long** AC_values, int* AC_event_count);

/* End recording counters with timestamp printed (useful for coarse-grain profiling).
   Adds the counters to the list of counters to be printed at the end. */
void EndRecordAriesCounters(int my_rank, int reporting_rank_mod, double run_time, int* AC_event_set, char*** AC_events, long long** AC_values, int* AC_event_count);

/* End recording counters WITHOUT timestamp printed (useful for fine-grain profiling).
   DOES NOT add the counters to the list of counters to be printed at the end;
   instead, returns the counters to the caller so that the caller may determine whether or not to save them.
   Caller should later call ReturnAriesCounters to discard them or PutAriesCounters to add them to the list to be printed. */
struct timestep_counters *EndRecordQuietAriesCounters(int my_rank, int reporting_rank_mod, double run_time, int* AC_event_set, char*** AC_events, long long** AC_values, int* AC_event_count);

/* Discard counters which shouldn't be printed at end (useful for fine-grain profiling, where it is impractical to save all counters) */
void ReturnAriesCounters(int my_rank, int reporting_rank_mod, struct timestep_counters *counters);

/* Add the counters to the list of counters which will definitely be printed at the end (only need to call this if EndRecordQuietAriesCounters was used) */
void PutAriesCounters(int my_rank, int reporting_rank_mod, struct timestep_counters *counters);

void FinalizeAriesCounters(MPI_Comm* mod16_comm, int my_rank, int reporting_rank_mod, int* AC_event_set, char*** AC_events, long long** AC_values, int* AC_event_count);

void WriteAriesCounters(int number_of_reporting_ranks, int reporting_rank_mod, long long *counter_data, char* nettilefile, char* proctilefile, char*** AC_events, int* AC_event_count);

#endif

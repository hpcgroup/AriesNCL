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

#define MAX_COUNTER_NAME_LENGTH 70

/* Functions for timing that use gettimeofday */
void StartSysTimer();
unsigned long long EndSysTimer();

/* Store counters in memory in a linked list, reporting them to rank 0 at the
 * end.
 */
struct timestep_counters {
    long long *counters;
    int timestep;
    unsigned long long elapsed_time; 
    struct timestep_counters *next;
};

/* library methods */
void ReadAriesCountersFile(char*** AC_events, int* AC_event_count);
void InitAriesCounters(char *progname, int my_rank, int reporting_rank_mod, int* AC_event_set, char*** AC_events, long long** AC_values, int* AC_event_count);

/* Start recording Aries counters for next region. */
void StartRecordAriesCounters(int my_rank, int reporting_rank_mod, int* AC_event_set, char*** AC_events, long long** AC_values, int* AC_event_count);

/* End recording counters for current region. Adds the counters to the list of
 * counters to be printed at the end.
 */
void EndRecordAriesCounters(int my_rank, int reporting_rank_mod, int* AC_event_set, char*** AC_events, long long** AC_values, int* AC_event_count);

void FinalizeAriesCounters(MPI_Comm* mod16_comm, int my_rank, int reporting_rank_mod, int* AC_event_set, char*** AC_events, long long** AC_values, int* AC_event_count);

void WriteAriesCounters(int number_of_reporting_ranks, int reporting_rank_mod, long long *counter_data, unsigned long long *timer_data, int timestep, char* jsonfile, char* binfile, char*** AC_events, int* AC_event_count);

#endif

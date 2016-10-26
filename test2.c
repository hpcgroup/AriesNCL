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

#include "AriesCounters.h"
#include <unistd.h> // sleep
#include <stdio.h>
#include <time.h>

#include "mpi.h"

extern struct timestep_counters *counters_list;

int main(int argc, char *argv[])
{
	int numtasks, taskid;
	MPI_Init(&argc, &argv);
	MPI_Comm_size(MPI_COMM_WORLD, &numtasks);
	MPI_Comm_rank(MPI_COMM_WORLD, &taskid);

	int AC_event_set;
	char** AC_events;
	long long * AC_values;
	int AC_event_count;

	// Since we only want to do a gather on every 16th rank, we need to create a new MPI_Group
	MPI_Group mod16_group, group_world;
	MPI_Comm mod16_comm;
	int members[2];
	int rank;
	for (rank=0; rank<2; rank++)
	{
		members[rank] = rank*16;
	}
	MPI_Comm_group(MPI_COMM_WORLD, &group_world);
	MPI_Group_incl(group_world, 2, members, &mod16_group);
	MPI_Comm_create(MPI_COMM_WORLD, mod16_group, &mod16_comm);

	InitAriesCounters(argv[0], taskid, 16, &AC_event_set, &AC_events, &AC_values, &AC_event_count);
       
        // Start recording, quiet version (no print statements).
	StartRecordQuietAriesCounters(taskid, 16, &AC_event_set, &AC_events, &AC_values, &AC_event_count);
	sleep(1);
        // Collect the counters, version which saves them. Then discard them.
        struct timestep_counters *counters;
	counters = EndRecordQuietAriesCounters(taskid, 16, 123.456, &AC_event_set, &AC_events, &AC_values, &AC_event_count);
        ReturnAriesCounters(taskid, 16, counters);

        if (taskid == 0) {
            printf("Counters returned pointer %p\n", counters);
            printf("%d: %lld %lld %lld %lld ...\n", counters->timestep, counters->counters[5], counters->counters[6],
									counters->counters[7], counters->counters[8]);
            printf("Counters to be printed (should be NULL): %p\n", counters_list);
        }

        // Do it again. This time save the counters to print on FinalizeAriesCounters.
	StartRecordQuietAriesCounters(taskid, 16, &AC_event_set, &AC_events, &AC_values, &AC_event_count);
	sleep(1);
	counters = EndRecordQuietAriesCounters(taskid, 16, 123.456, &AC_event_set, &AC_events, &AC_values, &AC_event_count);
        PutAriesCounters(taskid, 16, counters);

        if (taskid == 0) {
            printf("Counters returned pointer %p\n", counters);
            printf("%d: %lld %lld %lld %lld ...\n", counters->timestep, counters->counters[5], counters->counters[6],
									counters->counters[7], counters->counters[8]);
            printf("Counters to be printed:\n");
            printf("%d: %lld %lld %lld %lld ...\n", counters_list->timestep, counters_list->counters[5], counters_list->counters[6],
									counters_list->counters[7], counters_list->counters[8]);
        }

	FinalizeAriesCounters(&mod16_comm, taskid, 16, &AC_event_set, &AC_events, &AC_values, &AC_event_count);
	
	MPI_Finalize();

	return 0;
}

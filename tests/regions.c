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

#include <unistd.h> // sleep
#include <stdio.h>
#include <time.h>

#include "mpi.h"
#include "AriesCounters.h"


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

	// Run this many ranks per node in the test.
	int cpn = 64;
	// Run the test on this many nodes.
	int nodes = 2;

	// Since we only want to do a gather on every n'th rank, we need to create a new MPI_Group
	MPI_Group mod_group, group_world;
	MPI_Comm mod_comm;
	int members[nodes];
	int rank;
	for (rank=0; rank<nodes; rank++)
	{
		members[rank] = rank * cpn;
	}
	MPI_Comm_group(MPI_COMM_WORLD, &group_world);
	MPI_Group_incl(group_world, nodes, members, &mod_group);
	MPI_Comm_create(MPI_COMM_WORLD, mod_group, &mod_comm);

	int myrank;
	MPI_Comm_rank(MPI_COMM_WORLD, &myrank);
	int buf[5000];

	InitAriesCounters(argv[0], taskid, cpn, &AC_event_set, &AC_events, &AC_values, &AC_event_count);

	int i;
	for (i = 1; i <= 5; i++) {
		StartRecordAriesCounters(taskid, cpn, &AC_event_set, &AC_events, &AC_values, &AC_event_count);

		// Test proctile counters by sending i-thousand ints of data.
		int msgsize = i * 1000;
		if (myrank == 0) {
		    printf("Rank 0: sending %ld bytes, sleeping %d seconds\n", sizeof(int) * msgsize, i);
		    MPI_Send(buf, msgsize, MPI_INT, cpn, 0, MPI_COMM_WORLD);
		}
		if (myrank == cpn) {
		    MPI_Recv(buf, msgsize, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
		}

		// Test timing by sleeping i seconds.
		sleep(i);

		EndRecordAriesCounters(taskid, cpn, &AC_event_set, &AC_events, &AC_values, &AC_event_count);
	}

	FinalizeAriesCounters(&mod_comm, taskid, cpn, &AC_event_set, &AC_events, &AC_values, &AC_event_count);
	
	MPI_Finalize();

	return 0;
}

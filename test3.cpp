#include "AriesCounters.h"
#include <unistd.h> // sleep
#include <stdio.h>
#include <time.h>

#include "mpi.h"


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

	InitAriesCounters(taskid, 16, &AC_event_set, &AC_events, &AC_values, &AC_event_count);
	StartRecordAriesCounters(taskid, 16, &AC_event_set, &AC_events, &AC_values, &AC_event_count);
	
	sleep(10);

	EndRecordAriesCounters(&mod16_comm, taskid, 16, &AC_event_set, &AC_events, &AC_values, &AC_event_count);
	FinalizeAriesCounters(taskid, 16, &AC_event_set, &AC_events, &AC_values, &AC_event_count);
	
	MPI_Finalize();

	return 0;
}

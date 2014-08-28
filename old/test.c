
#include "mpi.h"
#include <papi.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "AriesCounters.h"
//#include "AriesCounters.hpp"

#define  MASTER		0

int main (int argc, char *argv[])
{
	int  numtasks, taskid, len;
	char hostname[MPI_MAX_PROCESSOR_NAME];
	int  partner, message;
	MPI_Status stats[2];
	MPI_Request reqs[2];

	MPI_Init(&argc, &argv);
	MPI_Comm_size(MPI_COMM_WORLD, &numtasks);
	MPI_Comm_rank(MPI_COMM_WORLD,&taskid);
	MPI_Get_processor_name(hostname, &len);
	//printf ("Hello from task %d on %s!\n", taskid, hostname);
	if (taskid == MASTER)
	   printf("MASTER: Number of MPI tasks is: %d\n",numtasks);

	int AC_event_set;
	char** AC_events;
	long long * AC_values;
	int AC_event_count;

	if (taskid%12 == 0)
	{
		StartRecordAriesCounters(&AC_event_set, &AC_events, &AC_values, &AC_event_count);
		//AriesCounters::StartRecording();
	}

	/* determine partner and then send/receive with partner */
	if (taskid < numtasks/2) 
	  partner = numtasks/2 + taskid;
	else if (taskid >= numtasks/2) 
	  partner = taskid - numtasks/2;

	int i;
	for (i=0;i<taskid+1;i++)
	{
		MPI_Irecv(&message, 1, MPI_INT, partner, 1, MPI_COMM_WORLD, &reqs[0]);
	}
	for (i=0;i<partner+1;i++)
	{
		MPI_Isend(&taskid, 1, MPI_INT, partner, 1, MPI_COMM_WORLD, &reqs[1]);
	}
	
	/* now block until requests are complete */
	MPI_Waitall(2, reqs, stats);

	/* print partner info and exit*/
	printf("Task %d is partner with %d\n",taskid,message);

	MPI_Finalize();
	if (taskid%12 == 0)
	{
		EndRecordAriesCounters(taskid, &AC_event_set, &AC_events, &AC_values, &AC_event_count);
		//AriesCounters::EndRecording(taskid);
	}
}

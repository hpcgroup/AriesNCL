#include "AriesCounters.h"
#include <unistd.h> // sleep
#include <stdio.h>
#include <time.h>

#include "mpi.h"

#define NANO_SECOND_MULTIPLIER  1000000  // 1 millisecond = 1,000,000 Nanoseconds

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

	StartRecordAriesCounters(&AC_event_set, &AC_events, &AC_values, &AC_event_count);
	
	int i = 0;
	FILE* fp = fopen("sampleData.txt", "w"); // clear file
	fclose(fp);
	fp = fopen("sampleData.txt", "w+");

	struct timespec tim;
	tim.tv_sec = 0;
	tim.tv_nsec = 500*NANO_SECOND_MULTIPLIER;

	for (i = 0; i < 10; i++)
	{
		nanosleep(&tim, NULL);
		fprintf(fp,"Sample %d\n", i);
		SampleAriesCounters(fp, &AC_event_set, &AC_events, &AC_values, &AC_event_count);
	}
	fclose(fp);
	
	//sleep(5);

	EndRecordAriesCounters(taskid, &AC_event_set, &AC_events, &AC_values, &AC_event_count);
	
	MPI_Finalize();

	return 0;
}

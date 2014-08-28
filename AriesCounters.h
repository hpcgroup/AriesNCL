#ifndef ARIESCOUNTERS_H
	#define ARIESCOUNTERS_H

#include <papi.h>
#include <stdio.h> // I/O
#include <stdlib.h> // Malloc, atoi
#include <string.h> // strcpy, strcat
#include <unistd.h> // sleep
#include <ctype.h> // isdigit

#include "mpi.h"

#define MAX_COUNTER_NAME_LENGTH 70

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
	
	PAPI_start(*AC_event_set);
}

// This is the value to put in the output file with format counterData-XXX.txt
void EndRecordAriesCounters(MPI_Comm* mod16_comm, int my_rank, int reporting_rank_mod, int* AC_event_set, char*** AC_events, long long** AC_values, int* AC_event_count)
{
	if (my_rank % reporting_rank_mod != 0) { return; }

	int number_of_reporting_ranks;
	// Array to store counter data
	long long * counter_data;

	PAPI_stop(*AC_event_set, *AC_values);

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

	/*
	int* s = (int*)malloc(sizeof(int)*2);
	s[0] = 1;
	s[1] = 3;
	int* store = (int*)malloc(sizeof(int) * 64);

	MPI_Gather(s, 2, MPI_INT, store, 2, MPI_INT, 0, MPI_COMM_WORLD);
	MPI_Barrier(MPI_COMM_WORLD);

	free(store);
	*/

	if (my_rank == 0)
	{
		int i,j;
		
		FILE* fp = fopen("counterData.yaml", "w");
		/* print out in yaml -- same format as in boxfish */
		fprintf(fp, "---\nkey: ARIESCOUNTER_ORIGINAL\n---\n");
		fprintf(fp, "- [mpirank, int32]\n");
		fprintf(fp, "- [tilex, int32]\n");
		fprintf(fp, "- [tiley, int32]\n");

		for (i=0; i<*AC_event_count; i++)
		{
			fprintf(fp, "- [%s, int128]\n", (*AC_events)[i]);
		}
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
				if (new_coord)
				{
					fprintf(fp, "\n%d %d %d", reporting_rank, x, y);
					new_coord = 0;
				}
				fprintf(fp, " %lld", counter_data[i * (*AC_event_count) + j]);
			}
		}
		fclose(fp);

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

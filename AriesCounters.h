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

#include <time.h>
void timestamp()
{
    time_t ltime; /* calendar time */
    ltime=time(NULL); /* get current cal time */
    printf("%s",asctime( localtime(&ltime) ) );

    struct timeval systime;
    gettimeofday(&systime, NULL);
    long stamp = systime.tv_sec * 1000 + systime.tv_usec / 1000.0;
    printf("gettimeofday: %ld\n", stamp);
}

#define MAX_COUNTER_NAME_LENGTH 70

/* counter for regions/timesteps profiled */
int region = 1;
/* store the name of the caller to write distinct files */
char *caller_program;
/* buffer for rank 0 to gather counters at the end */
long long *counter_data;

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

/* Store counters in memory in a linked list, reporting them to rank 0 at the end. */
struct timestep_counters {
	long long *counters;
	int timestep;
	struct timestep_counters *next;
};

struct timestep_counters *counters_list;

/* library methods */
void ReadAriesCountersFile(char*** AC_events, int* AC_event_count) {
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
}

void InitAriesCounters(char *progname, int my_rank, int reporting_rank_mod, int* AC_event_set, char*** AC_events, long long** AC_values, int* AC_event_count)
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

	ReadAriesCountersFile(AC_events, AC_event_count);

	*AC_values = (long long *)malloc(sizeof(long long) * (*AC_event_count));

	// Initialize PAPI
	PAPI_library_init(PAPI_VER_CURRENT);
	PAPI_create_eventset(AC_event_set);
	int code = 0;
	int i;
	for (i = 0; i < *AC_event_count; i++)
	{
		PAPI_event_name_to_code((*AC_events)[i], &code);
		PAPI_add_event(*AC_event_set, code);
	}

	int number_of_reporting_ranks;
	// Array to store counter data
	int size;
	MPI_Comm_size(MPI_COMM_WORLD, &size);
	number_of_reporting_ranks = (size-1)/reporting_rank_mod + 1; // integer division is fine; we want the floor

	// Ranks need to collect counters in memory.
	// Initialize the linked list here.
	counters_list = NULL;

	
	/* Rank 0 needs to write out counter info at the end.
	   First space needs to be allocated to recieve the data */
	// Array of longlong equal to number of reporting ranks * number of counters
	// Also set the program name here
	if (my_rank == 0)
	{
		counter_data = (long long*)malloc(sizeof(long long) * number_of_reporting_ranks * *AC_event_count);

		// We want to get just the executable name, not the whole directory if we have an absolute path
		char *exe_name;
		char *tok = strtok(progname, "/");
		while ( tok ) {
                  exe_name = tok;
		  tok = strtok(NULL, "/");
                }

		caller_program = (char *)malloc(sizeof(char) * (strlen(exe_name) + 1));
		strcpy(caller_program, exe_name);

		printf("Counters: Detected exe name: %s\n", caller_program);
	}
}

void StartRecordAriesCounters(int my_rank, int reporting_rank_mod, int* AC_event_set, char*** AC_events, long long** AC_values, int* AC_event_count)
{
	if (my_rank % reporting_rank_mod != 0) { return; }

	if (my_rank == 0) {	
	  printf("counters: Starting counters for timestep.\n");
	  timestamp();
	}

	PAPI_start(*AC_event_set);

	// Start a timer to measure elapsed time
	StartSysTimer();	
}

void EndRecordAriesCounters(int my_rank, int reporting_rank_mod, double run_time, int* AC_event_set, char*** AC_events, long long** AC_values, int* AC_event_count)
{
	if (my_rank % reporting_rank_mod != 0) { return; }

	if (my_rank == 0) { 
	  printf("counters: Collecting counters for timestep.\n");
	  timestamp();
	}

	// Stop timer for elapsed time
	EndSysTimer();

	PAPI_stop(*AC_event_set, *AC_values);
	PAPI_reset(*AC_event_set);

	// Store the counters in linked list in memory.
	struct timestep_counters *new_counters = (struct timestep_counters *)malloc(sizeof(struct timestep_counters));
	// Copy the counters.
	int num_bytes = sizeof(long long) * (*AC_event_count);
	new_counters->counters = (long long *)malloc(num_bytes);
	memcpy(new_counters->counters, *AC_values, num_bytes);

	// Store the region number.
	new_counters->timestep = region;

	// Insert at the head of the list for convenience.
	new_counters->next = counters_list;
	counters_list = new_counters;

	region++;

	if (my_rank == 0) { 
	  printf("counters: Done collecting.\n");
	  timestamp();
	}
}

void FinalizeAriesCounters(MPI_Comm* mod16_comm, int my_rank, int reporting_rank_mod, int* AC_event_set, char*** AC_events, long long** AC_values, int* AC_event_count)
{
	if (my_rank % reporting_rank_mod != 0) { return; }

	// Collect all the counters to rank 0 and dump them to binary files.
	int size;
	int number_of_reporting_ranks;
	MPI_Comm_size(MPI_COMM_WORLD, &size);
	number_of_reporting_ranks = (size-1)/reporting_rank_mod + 1; // integer division is fine; we want the floor

	// Walk the list of timestep counters, and for each one print out the data to a binary file.
	struct timestep_counters *ref = counters_list;
	while (ref) {

		/* MPI_Gather to collect counters from all ranks % 0 */
		MPI_Gather(ref->counters, *AC_event_count, MPI_LONG_LONG,
			counter_data, *AC_event_count, MPI_LONG_LONG, 0, *mod16_comm);

		/* Write out counter_data in binary, and read back in later to generate yaml. */
		if (my_rank == 0) {
			int timestep = ref->timestep;
			char filename[50];
			sprintf(filename, "%s.tiles.%d.bin", caller_program, timestep);
			FILE* fp = fopen(filename, "w");
			fwrite(counter_data, sizeof(long long), number_of_reporting_ranks * (*AC_event_count), fp);

			fclose(fp);
		}
		// Have everyone wait until rank 0 finishes, since it may take a while.
		MPI_Barrier(*mod16_comm);

		ref = ref->next;
	}

	// cleanup malloc
	int i;
	for (i = 0; i < *AC_event_count; i++)
	{
		free((*AC_events)[i]);
	}
	free(*AC_events);
	free(*AC_values);

	// Cleanup counter_data
	free(counter_data);
	free(caller_program);

	// Cleanup timestep lists
	ref = counters_list;
	while (ref) {
		struct timestep_counters *curr = ref;
		ref = ref->next;
		free(curr->counters);
		free(curr);
	}

	// Cleanup papi
	PAPI_cleanup_eventset(*AC_event_set);
	PAPI_destroy_eventset(AC_event_set);
	PAPI_shutdown();
}

void ReadAriesCountersBinary(int number_of_reporting_ranks, int reporting_rank_mod, char* bin_filename, char*** AC_events, int* AC_event_count) {
	// Allocate buffer to read back into
	counter_data = (long long*)malloc(sizeof(long long) * number_of_reporting_ranks * *AC_event_count); 

	// Read back the counters from the binary file
	FILE* fp = fopen(bin_filename, "r");
	fread(counter_data, sizeof(long long), number_of_reporting_ranks * *AC_event_count, fp);
	fclose(fp);

	// Expect that bin_filename is format: "exe_name.tiles.%d.bin"
	char* temp = (char *)malloc(sizeof(char) * strlen(bin_filename));  // Save a reference to free this after.
	strcpy(temp, bin_filename);

        char* region = temp;
        while (*region != '.') {
        	region++;
        }
        *region = '\0';  // Want to extract exe_name, so set end as null.
        // Region should now point right before "tiles".
	region = region + 7;
	char* ref = region;
	while( *ref != '.' ) {
		ref++;
	}
	*ref = '\0';  // This extracts the timestep.

	int timestep = atoi(region);
        char *exe_name = temp;

	char filename[50];
	sprintf(filename, "%s.networktiles.%d.yaml", exe_name, timestep);

	//free(temp);

	int i,j;
	fp = fopen(filename, "w");
	/* print out in yaml -- same format as in boxfish */
	fprintf(fp, "---\nkey: ARIESCOUNTER_NETWORK\n---\n");
	fprintf(fp, "- [mpirank, int32]\n");
	fprintf(fp, "- [tilex, int32]\n");
	fprintf(fp, "- [tiley, int32]\n");

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

	sprintf(filename, "%s.proctiles.%d.yaml", exe_name, timestep);
	fp = fopen(filename, "w");

	/* print out in yaml -- same format as in boxfish */
	fprintf(fp, "---\nkey: ARIESCOUNTER_PROC\n---\n");
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
	free(temp);

}

#endif

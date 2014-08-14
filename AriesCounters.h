#ifndef ARIESCOUNTERS_H
	#define ARIESCOUNTERS_H

#include <papi.h>
#include <stdio.h> // I/O
#include <stdlib.h> // Malloc
#include <string.h> // strcpy, strcat
#include <unistd.h> // sleep

#define MAX_COUNTER_NAME_LENGTH 70

void StartRecordAriesCounters(int* AC_event_set, char*** AC_events, long long** AC_values, int* AC_event_count)
{
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

	//printf("Linecount: %d\n", linecount);

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
	PAPI_start(*AC_event_set);

	// The counters don't seem to report numbers if we start right away
	//sleep(1);
}

void SampleAriesCounters(FILE* fp, int* AC_event_set, char*** AC_events, long long** AC_values, int* AC_event_count)
{
	PAPI_stop(*AC_event_set, *AC_values);
	
	int i = 0;
	for (i = 0; i < *AC_event_count; i++)
	{
		fprintf(fp, "%s %lld\n", (*AC_events)[i], (*AC_values)[i]);
	}

	PAPI_start(*AC_event_set);
}

// This is the value to put in the output file with format counterData-XXX.txt
void EndRecordAriesCounters(int preAppend, int* AC_event_set, char*** AC_events, long long** AC_values, int* AC_event_count)
{
	// Similar to the sleep in startrecord
	//sleep(1);

	PAPI_stop(*AC_event_set, *AC_values);

	// write out counter data
	char filename[70] = "counterData-";
	char preAppend_str[10];
	sprintf(preAppend_str, "%d", preAppend); // int to char*
	strcat(filename, preAppend_str);
	strcat(filename, ".txt");

	FILE* fp = fopen(filename, "w");
	int i;
	// In order to save space (since these files grow fast), the counter names are not shown
	// We refer to them by the line number from the original counters.txt

	for (i = 0; i < *AC_event_count; i++)
	{
		// I think %lld is not portable
		//fprintf(fp, "%i %lld\n", i, (*AC_values)[i]);
		fprintf("%s %lld\n", AC_events[i], AC_values[i]);
	}
	fclose(fp);

	// cleanup malloc
	// Not working with MPI right now...? Some sort of bug
	for (i = 0; i < *AC_event_count; i++)
	{
		//free((*AC_events)[i]);
	}
	//free(*AC_events);
	//free(*AC_values);

	PAPI_cleanup_eventset(*AC_event_set);
	PAPI_destroy_eventset(AC_event_set);
	PAPI_shutdown();
}

#endif

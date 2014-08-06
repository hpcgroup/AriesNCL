#include "AriesCounters.h"
#include <unistd.h> // sleep
#include <stdio.h>
#include <time.h>

#define NANO_SECOND_MULTIPLIER  1000000  // 1 millisecond = 1,000,000 Nanoseconds

int main(int argc, char const *argv[])
{
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
	tim.tv_nsec = 100*NANO_SECOND_MULTIPLIER;

	for (i = 0; i < 50; i++)
	{
		nanosleep(&tim, NULL);
		fprintf(fp,"Sample %d\n", i);
		SampleAriesCounters(fp, &AC_event_set, &AC_events, &AC_values, &AC_event_count);
	}
	fclose(fp);
	

	//sleep(5);

	EndRecordAriesCounters(1, &AC_event_set, &AC_events, &AC_values, &AC_event_count);
	return 0;
}

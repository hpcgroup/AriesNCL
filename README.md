###Aries Hardware Counters

A pretty simple header file for everything related to Aries Hardware counters. The code does not currently support configurable counters, though the API through PAPI is there.

For C (AriesCounters.h):

 * void InitAriesCounters(int my_rank, int reporting_rank_mod, int* AC_event_set, char*** AC_events, long long** AC_values, int* AC_event_count)
    * Call this first! This initializes PAPI and sets up the counters. It expects a file called 'counters.txt' in the same directory as the executable with a newline-delimeted list of counter names to record
 * void StartRecordAriesCounters(int my_rank, int reporting_rank_mod, int* AC_event_set, char*** AC_events, long long** AC_values, int* AC_event_count)
    * Starts recording counters.
 * void EndRecordAriesCounters(MPI_Comm* mod16_comm, int my_rank, int reporting_rank_mod, double run_time, int* AC_event_set, char*** AC_events, long long** AC_values, int* AC_event_count)
    * Ends recording counters. Writes out to two YAML files (networktiles.yaml and proctiles.yaml). See boxfish for format
 * void FinalizeAriesCounters(int my_rank, int reporting_rank_mod, int* AC_event_set, char*** AC_events, long long** AC_values, int* AC_event_count)
    * Cleans up memory, stops PAPI.

-Depreciated!- I suggest using the C version
For C++ (AriesCounters.hpp, AriesCounters.cpp):

 * void AriesCounters::StartRecording()
 * void AriesCounters::EndRecording(int preAppend)

Note: Depending on the compiler/environment you run this in, it is possible to compile the C++ version for C code (try including the LFLAGS). Works on Edison@NERSC's login nodes but not compute nodes in PrgEnv-intel.

####To Compile
The standard Makefile should be enough. If you only want to compile the library file, type 'Make libariescounters.a'

Make sure module papi is loaded before compiling. You may also need to unload the darshan module.

####Running tests
The main test is test3.cpp, which creates mpitest.out. It is a MPI program. Within folder 'old' are other tests including non-MPI ones.
To setup the variables required by the function calls look at test3.cpp or the following:

	MPI_Group mod16_group, group_world;
	MPI_Comm mod16_comm;
	int members[2];
	int rank;
	for (rank=0; rank<2; rank++)
	{
		members[rank] = rank*16; // Change this to your reporting_rank_mod
	}
	MPI_Comm_group(MPI_COMM_WORLD, &group_world);
	MPI_Group_incl(group_world, 2, members, &mod16_group);
	MPI_Comm_create(MPI_COMM_WORLD, mod16_group, &mod16_comm);

Since every node will record the same counter information, we only record it on one rank per node (we could have up to 3 redudant records on Edison but we cannot easily figure out how many nodes we own on a router). The reporting_rank_mod is the number of ranks per node.

These must be run on nodes with papi support (ie Cray's NPU component). All of Edison's compute nodes have this.

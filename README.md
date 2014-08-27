###Aries Hardware Counters

Includes both a C and C++ version. To change, just uncomment the target for your language in the Makefile and update the 2 function calls in test.c, test2.cpp.

For C (AriesCounters.h):

 * void StartRecordAriesCounters()
 * void EndRecordAriesCounters(int preAppend)

-Depreciated!- I suggest using the C version
For C++ (AriesCounters.hpp, AriesCounters.cpp):

 * void AriesCounters::StartRecording()
 * void AriesCounters::EndRecording(int preAppend)

Note: Depending on the compiler/environment you run this in, it is possible to compile the C++ version for C code (try including the LFLAGS). Works on Edison@NERSC's login nodes but not compute nodes in PrgEnv-intel.

####To Compile
The standard Makefile should be enough. If you only want to compile the library file, type 'Make libariescounters.a'

Make sure module papi is loaded before compiling. You may also need to unload the darshan module.

####Running tests
There are two test programs in the repo. Make will create them (solo.out and counter.out)
The files are hardcoded to read which counters to track from a file called 'counters.txt'. Make sure this file has an empty line at the end of the program.
These must be run on nodes with papi support (ie Cray's NPU component). All of Edison's compute nodes have this.
counter.out is an MPI program and must be ran with aprun (at least 2 cores). The counter data is dumped into files with the format counterData-##.txt where the number is the MPI rank.

####Cleanup
The C code is made to be lightweight and fast. The output is not terribly user-friendly. There is a script called cleanup.py that will make the data easier to work with. Just give it as an argument the top level folder which all runs are contained (or the path to a single run's folder). It will take the files and generate a JSON file with all the data, indexed by mpi-rank, then by an XY string representing the coordinates of the network tile. The values are a list of integers, representing the counter values. They are in order, alphanumerically the same way the counters.txt file is listed.

###Aries Hardware Counters

Includes both a C and C++ version. To change, just uncomment the target for your language in the Makefile and update the 2 function calls in test.c, test2.cpp.

For C (AriesCounters.h):

 * void StartRecordAriesCounters()
 * void EndRecordAriesCounters(int preAppend)

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

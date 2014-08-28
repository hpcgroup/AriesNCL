#ifndef ARIES_COUNTERS_H
	#define ARIES_COUNTERS_H

#include <papi.h>
#include <vector>
#include <string>
#include <fstream> // ifstream
#include <iostream> // cout
#include <unistd.h> // sleep
#include <sstream> // ostringstream
#include <iostream> // ios

// A container to hold the operations of the Aries' hardware counters
// It is static since having more than one eventset active is undefined behavior
// Easiest way to run on multiple nodes is just to do aprun and specify which nodes using -cc
// It doesn't make much sense to run this on nodes under the same Aries router.
// The values will be the same (or sometimes off by a few flits)
// The counters that will be recorded should be listed (one per line) in a file named "counters.txt"
// The counter data will be written out to a file named "counterData-XXX.txt"
// where XXX is the value of preAppend
class AriesCounters
{
public:
	static void StartRecording();
	static void EndRecording(int preAppend);

private:
	static int eventSet;
	static long long * values;
	static std::vector<std::string> events;
};

#endif

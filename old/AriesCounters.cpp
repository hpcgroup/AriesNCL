#include "AriesCounters.hpp"

using namespace std;

int AriesCounters::eventSet;
long long * AriesCounters::values;
vector<string> AriesCounters::events;

void AriesCounters::StartRecording()
{
	eventSet = PAPI_NULL;

	// Read the list of counters into events
	string line;
	ifstream f("counters.txt");
	if (f)
	{
		while (getline(f,line))
		{
			events.push_back(line);
		}
		f.close();
	}
	values = new long long[events.size()];

	// Initialize PAPI
	PAPI_library_init(PAPI_VER_CURRENT);
	PAPI_create_eventset(&eventSet);
	int code = 0;
	for (vector<string>::iterator i = events.begin(); i != events.end(); i++)
	{
		// Requires a writable char* from string
		char* eventName = new char[i->size() + 1];
		copy(i->begin(), i->end(), eventName);
		eventName[i->size()] = '\0';

		PAPI_event_name_to_code(eventName, &code);
		PAPI_add_event(eventSet, code);

		delete[] eventName;
	}
	PAPI_start(eventSet);

	// The counters don't seem to report numbers if we start right away
	sleep(1);
}

void AriesCounters::EndRecording(int preAppend)
{
	// Similar to start's sleep
	sleep(1);

	PAPI_stop(eventSet, values);
	// print counter data
	ofstream outfile;
	ostringstream filename;
	filename << "counterData-" << preAppend << ".txt";
	cout << "writing out to " << filename.str().c_str() << endl;

	outfile.open(filename.str().c_str());

	for (int i=0; i< events.size(); i++)
	{
		//cout << events[i] << " = " << values[i] << endl;
		outfile << events[i] << " = " << values[i] << endl;
	}
	outfile.close();

	delete[] values;

	PAPI_cleanup_eventset(eventSet);
	PAPI_destroy_eventset(&eventSet);
	PAPI_shutdown();
}

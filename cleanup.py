# Given a folder path of counter output, this script will generate a json file
# named 'counterData.json'. Each entry is the counter name, and the value is
# a list of counter values in order of nodes reporting. (typically every 16th)

import os
import sys
import json

from pprint import pprint

def cleanup(counter_folder_path, counters_txt_path):

	to_dump = dict()

	# first read counters.txt for the names of the counters and setup the dict with them
	counter_list = [line.strip() for line in open(counters_txt_path)]
	for counter_name in counter_list:
		to_dump[counter_name] = list()

	maxNum = 0

	for f in os.listdir(counter_folder_path):
		if f.startswith('counterData-'):
			#process this
			counter_values = [line.strip() for line in open(counter_folder_path+'/'+f)]
			for counter_entry in counter_values:
				# looks like 2 numbers seperated by a space
				split = counter_entry.split()
				counter_name = split[0]
				counter_value = int(split[1])
				# NOTE: values can reach longlong but I've only seen 10^10 or so

				to_dump[counter_name].append(counter_value)

	# write out
	with open('counterData.json', 'w') as outfile:
		json.dump(to_dump, outfile)


def test():
	cleanup(sys.argv[1], sys.argv[2])

if __name__ == "__main__":
	test()
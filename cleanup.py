# Given a folder path of counter output, this script will generate a json file
# named 'counterData.json'. Each entry is the counter name, and the value is
# a list of counter values in order of nodes reporting. (typically every 16th)

# BE VERY SURE YOU ARE RUNNING THIS CORRECTLY. IT WILL DELETE YOUR HARDWARE COUNTER LOGS

import os
import sys
import json

import zipfile
import zlib

# DEPRECIATED
def cleanup(counter_folder_path, counters_txt_path):

	# check if a counterData.json exists (been processed before)
	for filename in os.listdir(counter_folder_path):
		if filename.endswith('counterData.json'):
			print '  No work needs to be done'
			return

	to_dump = dict()

	# first read counters.txt for the names of the counters and setup the dict with them
	counter_list = [line.strip() for line in open(counters_txt_path)]
	for counter_name in counter_list:
		to_dump[counter_name] = list()

	for f in os.listdir(counter_folder_path):
		if f.startswith('counterData-'):
			#process this
			counter_values = [line.strip() for line in open(counter_folder_path+'/'+f)]
			for counter_entry in counter_values:
				# looks like 2 numbers seperated by a space
				split = counter_entry.split()

				counter_name = split[0]
				counter_value = int(split[1])
				
				# if 

				# NOTE: values can reach longlong but I've only seen 10^10 or so
				to_dump[counter_name].append(counter_value)

	# write out
	with open(counter_folder_path + '/counterData.json', 'w') as outfile:
		json.dump(to_dump, outfile)

	# remove the counterData-* files
	[ os.remove(counter_folder_path + '/' + f) for f in os.listdir(counter_folder_path) if f.startswith('counterData-')]

def CleanupAllDatasets(rootdir):
	list_of_counterdata_folders = set()
	for subdir, dirs, files in os.walk(rootdir):
		#print 'examine', subdir
		for subdir2, dirs2, files2 in os.walk(subdir):
			#print subdir2
			if subdir2.endswith('counterData'):
				list_of_counterdata_folders.add(subdir2)

	for folder in list_of_counterdata_folders:
		print 'Cleaning up', folder
		#cleanup(folder, folder + '/../counters.txt')
		CompressFolder(folder)

def CompressFolder(folder):
	zf = zipfile.ZipFile('CounterData.zip', mode='w')
	for filename in os.listdir(folder):
		if filename.startswith('counterData-'):
			zf.write(filename, compress_type=zipfile.ZIP_DEFLATED)
	zf.close()

if __name__ == "__main__":
	if len(sys.argv) != 2:
		print 'Need 1 argument'
		sys.exit(1)
	CleanupAllDatasets(sys.argv[1])

# Given a folder path of counter output, this script will generate a json file
# named 'counterData.json'. Each entry is the counter name, and the value is
# a list of counter values in order of nodes reporting. (typically every 16th)

# BE VERY SURE YOU ARE RUNNING THIS CORRECTLY. IT WILL DELETE YOUR HARDWARE COUNTER LOGS

# TODO: Make ToJson faster?

import os
import sys
import json

import zipfile
import zlib

import warnings

from pprint import pprint

def cleanup(counter_folder_path, counters_txt_path):
	warnings.warn('This function is no longer used', DeprecationWarning)

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

	# Now go through all the counter files
	for f in os.listdir(counter_folder_path):
		if f.startswith('counterData-'):
			#process this
			counter_values = [line.strip() for line in open(counter_folder_path+'/'+f)]
			for counter_entry in counter_values:
				# looks like "COUNTER_NAME VALUE"
				split = counter_entry.split()

				counter_name = split[0]
				counter_value = int(split[1])

				# NOTE: values can reach longlong but I've only seen 10^10 or so
				to_dump[counter_name].append(counter_value)

	# write out
	with open(counter_folder_path + '/counterData.json', 'w') as outfile:
		json.dump(to_dump, outfile)

	# remove the counterData-* files
	[ os.remove(counter_folder_path + '/' + f) for f in os.listdir(counter_folder_path) if f.startswith('counterData-')]

def ToJson(folder, counters_txt_path):
	"""Takes all the counterData-* files in folder and converts them to YAML"""
	# first read counters.txt for the names of the counters and setup the dict with them
	counter_list = [line.strip() for line in open(counters_txt_path)]

	list_of_counterdata_files = list()
	for f in os.listdir(folder):
		if f.startswith('counterData-'):
			list_of_counterdata_files.append(folder+'/'+f)

	# eventually to dump to YAML. Indexed by mpi-rank, then XY as string of network tile coords
	to_dump = dict()

	for counter_file in list_of_counterdata_files:
		file_data = [line.strip() for line in open(counter_file)]
		mpi_rank = int(counter_file.split('-')[-1].split('.')[0])
		to_dump[mpi_rank] = dict()
		for x in range(6):
			for y in range(8):
				to_dump[mpi_rank]['%d%d' % (x,y)] = list()
		for line in file_data:
			split = line.split()
			counter_name = counter_list[int(split[0])]
			tile_XY = None
			if counter_name.startswith('AR_RTR_PT_'):
				tile_XY = counter_name[10] + counter_name[12]
			else: # stars with 'AR_RTR'
				tile_XY = counter_name[7] + counter_name[9]

			#if tile_XY not in to_dump[mpi_rank].keys():
			#	to_dump[mpi_rank][tile_XY] = dict()
			to_dump[mpi_rank][tile_XY].append(int(split[1]))

		# each to_dump[mpi_rank][tile_XY] is a dict right now.
		# We only need a list of values (no keys), but sorted by key alphanumerically
		#for XY in to_dump[mpi_rank].keys():
		#	to_dump[mpi_rank][XY] = [value for (key, value) in sorted(to_dump[mpi_rank][XY].items())]

		outfile = open(folder + '.json', 'w')
		outfile.write(json.dumps(to_dump))
		outfile.close()

def CleanupAllDatasets(rootdir):
	list_of_counterdata_folders = set()
	for subdir, dirs, files in os.walk(rootdir):
		for subdir2, dirs2, files2 in os.walk(subdir):
			if subdir2.endswith('counterData'):
				list_of_counterdata_folders.add(subdir2)

	for folder in list_of_counterdata_folders:
		print 'Cleaning up', folder
		#cleanup(folder, folder + '/../counters.txt')
		ToJson(folder, folder + '/../counters.txt')
		CompressFolder(folder)
		RemoveCounterFiles(folder)

def CompressFolder(folder):
	os.chdir(folder)
	if os.path.isfile('CounterData.zip'):
		print 'Nothing to be done'
		return
	zf = zipfile.ZipFile('CounterData.zip', mode='w')
	for filename in os.listdir(folder):
		if filename.startswith('counterData-'):
			zf.write(filename, compress_type=zipfile.ZIP_DEFLATED)
	zf.close()

def RemoveCounterFiles(folder):
	os.chdir(folder)
	# remove the counterData-* files
	[ os.remove(f) for f in os.listdir(folder) if f.startswith('counterData-')]

if __name__ == "__main__":
	if len(sys.argv) != 2:
		print 'Need 1 argument: rootdir for folders to cleanup'
		sys.exit(1)
	CleanupAllDatasets(sys.argv[1])

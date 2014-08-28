lines = [line.strip() for line in open("sampleData.txt")]

counterData = {}

counterNames = [
'AR_RTR_0_0_INQ_PRF_INCOMING_PKT_VC0_FILTER_FLIT0_CNT',
'AR_RTR_0_0_INQ_PRF_INCOMING_PKT_VC1_FILTER_FLIT1_CNT',
'AR_RTR_0_0_INQ_PRF_INCOMING_PKT_VC2_FILTER_FLIT2_CNT',
'AR_RTR_0_0_INQ_PRF_INCOMING_PKT_VC3_FILTER_FLIT3_CNT',
'AR_RTR_0_0_INQ_PRF_INCOMING_PKT_VC4_FILTER_FLIT4_CNT',
'AR_RTR_0_0_INQ_PRF_INCOMING_PKT_VC5_FILTER_FLIT5_CNT',
'AR_RTR_0_0_INQ_PRF_INCOMING_PKT_VC6_FILTER_FLIT6_CNT',
'AR_RTR_0_0_INQ_PRF_INCOMING_PKT_VC7_FILTER_FLIT7_CNT']

for line in lines:
	sampleNum = 0
	if line.startswith('Sample'):
		sampleNum = int(line.split(' ')[1])
		#print sampleNum
	else:
		linesplit = line.split(' ')
		counterName = linesplit[0]
		counterValue = long(linesplit[1])
		if counterName not in counterData.keys():
			counterData[counterName] = []
		counterData[counterName].append(counterValue)
		#if counterName == 'AR_RTR_0_1_INQ_PRF_INCOMING_FLIT_VC6':
		#	print counterValue

#for name, value in counterData.iteritems():
#	if name == 'AR_RTR_0_0_INQ_PRF_INCOMING_FLIT_VC0':
#		print counterData['AR_RTR_0_0_INQ_PRF_INCOMING_FLIT_VC0']

for name in counterNames:
	for val in counterData[name]:
		print name, val

#for name, value in counterData.iteritems():
#	print name

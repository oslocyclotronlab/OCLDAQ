from numpy import *

outfile = open("auto_setup.c", "w")

outfile.write("#include experimentsetup.h\n")
outfile.write("\n\n\n\n")

outfile.write("DetectorInfo_t pDetector[] =\n{\n")

num_channels = 176
first_labr_module_slot = 2
first_Si_module_slot = 5

listOfDetectors = empty(num_channels, dtype=list)

# Set everything to "zero"
for i in range(0, num_channels):
	listOfDetectors[i] = [i, "f000MHz", "unused", 0, 0]

labr_start = 32
labr_stop = 32+16*2

de_start = labr_stop
de_stop = de_start + 64

e_start = de_stop
e_stop = e_start + 8

# Set LaBr channels (we will figure out about this in more detail later)
for i in range(labr_start, labr_stop):
	listOfDetectors[i] = [i, "f500MHz", "labr", i-labr_start, 0]

# Set de channels
tel = 0
for i in range(de_start, de_stop):
	if i - de_start - 8*tel >= 8: tel += 1
	listOfDetectors[i] = [i, "f250MHz", "deDet", i-de_start, tel]

# Set e channels
for i in range(8):
	listOfDetectors[2*i + e_start] = [2*i+e_start, "f250MHz", "eGuard", i, 0]
	listOfDetectors[2*i + e_start+1] = [2*i+e_start+1, "f250MHz", "eDet", i, 0]



for i in range(0, num_channels):
	det = listOfDetectors[i]
	outfile.write("\t{%d, %s, %s, %d, %d},\n" % (det[0], det[1], det[2], det[3], det[4]) )

outfile.write("};\n")

outfile.close()


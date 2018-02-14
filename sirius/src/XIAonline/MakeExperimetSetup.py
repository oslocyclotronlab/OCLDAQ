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
	listOfDetectors[i] = [i, "f000MHz", "unused", 0]

# Set LaBr channels (we will figure out about this in more detail later)
for i in range(32, 32+16*3):
	listOfDetectors[i] = [i, "f500MHz", "labr", i-32]

# Set de channels
for i in range(32+16*3, 32+16*3+64):
	listOfDetectors[i] = [i, "f250MHz", "deDet", i-(32+16*3)]

# Set e channels
for i in range(32+16*3+64, 32+16*3+64+8):
	listOfDetectors[i] = [i, "f250MHz", "deDet", i-(32+16*3+64)]

# Set e guard channels
for i in range(32+16*3+64+8, 32+16*3+64+8+8):
	listOfDetectors[i] = [i, "f250MHz", "deDet", i-(32+16*3+64+8)]


for i in range(0, num_channels):
	det = listOfDetectors[i]
	outfile.write("\t{%d, %s, %s, %d},\n" % (det[0], det[1], det[2], det[3]) )

outfile.write("};\n")

outfile.close()


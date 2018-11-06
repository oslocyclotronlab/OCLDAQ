#!/usr/bin/env python
# -*- coding: utf-8 -*-

import os, sys

# First we will get the new experiment name from command line.
if len(sys.argv) != 2:
	print "Wrong command line arguments"
	sys.stdout.write("1")
	exit()

text = "#!/bin/bash\n"
text += "\n"
text += "cd /mnt/current/%s\n" % sys.argv[1]
text += "acq_master"

outfile = open("start.sh", "w")
outfile.write(text)
outfile.close()

# Make sure the start script is executable.
os.system("chmod +x start.sh")

# Now we are done :)
sys.stdout.write("0")
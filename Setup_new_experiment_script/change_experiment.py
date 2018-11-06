#!/usr/bin/env python
# -*- coding: utf-8 -*-

import os
import subprocess

def CheckDAQ_computer(ip_address):
	res = os.system("nc -z %s 22" % ip_address)
	if res != 0:
		return False
	return True

def CheckIfDAQIsRunning(ip_address):
	# SSH into the DAQ computer to check if any of the DAQ softwares are running!
	daq_command = "/usr/sbin/pidof XIAengineControl"
	ssh = subprocess.Popen(["ssh", "%s" % ip_address, daq_command],
					   shell=False,
					   stdout=subprocess.PIPE,
					   stderr=subprocess.PIPE)
	result = ssh.stdout.readlines()
	if len(result) > 0:
		return True # DAQ is running.
	else:
		return False

basedir = "/d8/exp/"
template_files = ["acq_master_commands.txt",
				  "input_output.sh",
				  "settings_coincidence.set",
				  "settings_singles.set",
				  "settings.set"]
daq_IP_address = "192.168.0.209"
daq_command = "./update_start_script.py %s"

test_daq = False
if test_daq:
	if not CheckDAQ_computer(daq_IP_address):
		print "DAQ computer not running, please start before running this tool."
		exit()

print "This tool changes the experiment that will be running."

exp_name = raw_input("Enter experiment name: ")
exp_path = basedir + exp_name

if not os.path.isdir(exp_path):
	print "Experiment does not exist. Please try again."
	exit(1)

# Check that all the files that are needed are really present!
is_good = True
for file in template_files:
	if not os.path.exists("%s/%s" % (exp_path, file) ):
		print "Experiment folder seems to be corrupted, could not find the %s file."
		print "Please copy the %s file from the template folder."

# Next step is to change the DAQ computer settings.
# Now we will have to ssh to the DAQ and set the new experiment name (hard part!)
ssh = subprocess.Popen(["ssh", "%s" % daq_IP_address, daq_command % exp_name],
					   shell=False,
					   stdout=subprocess.PIPE,
					   stderr=subprocess.PIPE)
result = ssh.stdout.readlines()
print result
print "Congratulations! We have successfully updated all settings and you may now start the DAQ."
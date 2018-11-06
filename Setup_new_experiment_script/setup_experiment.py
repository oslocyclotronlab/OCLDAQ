#!/usr/bin/env python
# -*- coding: utf-8 -*-

import os
import sys
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

# Settings for the script. Need to be changed if default storage place is moved.
basedir = "/d8/exp/"
templatedir = basedir + "template"
template_files = ["acq_master_commands.txt",
				  "input_output.sh",
				  "settings_coincidence.set",
				  "settings_singles.set",
				  "change_mode.py",
				  "pxisys.ini"]
elog_command = "elog -h localhost -p 8080 -l %s -a Type=Automatic -a Category=ACQ -a Author=GUI\n"
daq_IP_address = "192.168.0.209"
daq_command = "./update_start_script.py %s"
test_daq = False
if test_daq:
	if not CheckDAQ_computer(daq_IP_address):
		print "DAQ computer not running, please start before continuing."
		exit()
	if CheckIfDAQIsRunning(daq_IP_address):
		print "DAQ software is running. Please close all DAQ instances!"
		exit()

print "Welcome to the new experiment setup script."
print "I will now ask you a couple of questions."

## We will ask the user a couple of questions about the experiment.
## First we will ask what the experiment should be called.
print "Please enter the name of the experiment."
print "The name has to be the same as in the one given in the ELOG for automatic ELOG to work."

exp_name = raw_input("Name of experiment: ")
exp_path = basedir + exp_name
while 1:
	if os.path.isdir(exp_path):
		print "Experiment already exists, please use another name."
		exp_name = raw_input("Name of experiment: ")
		exp_path = basedir + exp_name
	else:
		break

print "Will you run the DAQ in singles or coincidence mode?"
print "1: Coincidence mode"
print "2: Singles mode"
coinc_mode = 1
try:
	coinc_mode = int(raw_input( "Select mode [default 1]: " ) )
except:
	coinc_mode = 1

if coinc_mode != 1 and coinc_mode != 2:
	print "Something went wrong... Why is coinc_mode=%d?" % coinc_mode
	exit(1)

# Next we will ask the user to review all the information before confirming the setup.
print "We will now copy and update all the different files needed!"

# Now we will check that there is no other experiments with that name on the /d8 drive.
# If there is no other, we will create a new folder on the /d8 drive.

# Create the folder.
os.system("mkdir %s" % exp_path)

# Next we will create the folder and copy files from the template folder.
for file in template_files:
	os.system( "cp %s/%s %s/%s" % (templatedir, file, exp_path, file) )

# Now we will open the acq_master_commands and add the elog line.
acq_commands_file = open("%s/%s" % (exp_path, "acq_master_commands.txt"), "a")
acq_commands_file.write(elog_command % exp_name)
acq_commands_file.close()

# Set the symlink to the correct settings file.
if coinc_mode == 1:
	os.system("rm -f %s/settings.set" % exp_path)
	os.system("ln -s %s/settings_coincidence.set %s/settings.set" % (exp_path, exp_path) )
elif coinc_mode == 2:
	os.system("rm -f %s/settings.set" % exp_path)
	os.system("ln -s %s/settings_singles.set %s/settings.set" % (exp_path, exp_path) )

# Now we will have to ssh to the DAQ and set the new experiment name (hard part!)
ssh = subprocess.Popen(["ssh", "%s" % daq_IP_address, daq_command % exp_name],
					   shell=False,
					   stdout=subprocess.PIPE,
					   stderr=subprocess.PIPE)
result = ssh.stdout.readlines()
if int(result) == 1:
	print "Problem updating the experiment. Please try ssh manually."
else:
	print "Congratulations! We have successfully updated all settings and you may now start the DAQ."

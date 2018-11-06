#!/usr/bin/env python
# -*- coding: utf-8 -*-

import os, sys
import subprocess

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

def CheckSysArg(args):
	if len(args) == 2:
		try:
			val = int(args[1])
			if val == 1 or val == 2:
				return val
			else:
				return 0
		except:
			return 0
	return 0

def AskUser():
	print "Will you run the DAQ in singles or coincidence mode?"
	print "1: Coincidence mode"
	print "2: Singles mode"
	coinc_mode = 1
	try:
		coinc_mode = int(raw_input( "Select mode [default 1]: " ) )
	except:
		coinc_mode = 1
	if coinc_mode != 1 and coinc_mode != 2:
		return 1
	return coinc_mode

# First check if we have a command line argument, if that is either 1 or 2, we will change the coincidence mode
# without asking the user anything.
# First check that DAQ is not running.
if CheckIfDAQIsRunning("192.168.0.209"):
	print "DAQ is running. Please stop DAQ and try again."
	exit()

coinc_mode = CheckSysArg(sys.argv)
if coinc_mode == 0:
	coinc_mode = AskUser()

# Delete and create new symlink.
if coinc_mode == 1:
	os.system("rm -f settings.set")
	os.system("ln -s settings_coincidence.set settings.set")
elif coinc_mode == 2:
	os.system("rm -f settings.set")
	os.system("ln -s settings_singles.set settings.set")
else:
	print "Something went wrong... Why is coinc_mode=%d?" % coinc_mode
	exit(1)

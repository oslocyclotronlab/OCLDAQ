#!/usr/bin/env python
# -*- coding: utf-8 -*-

import time, random, os, sys
import serial

verbose = False
#verbose = True

class MRCC_Exception(Exception):
    pass

###############################################################################
###############################################################################

class MRCC_Connection:

    ###########################################################################
    def __init__(self, device):
        #self.baudrate = 9600
        #self.baudrate = 19200
        #self.baudrate = 38400
        self.baudrate = 57600
        #self.baudrate = 115200
        self.serial = serial.Serial(device,
                                    baudrate=self.baudrate, #baudrate
                                    bytesize=serial.EIGHTBITS,    #number of databits
                                    parity=serial.PARITY_NONE,    #enable parity checking
                                    stopbits=serial.STOPBITS_ONE, #number of stopbits
                                    timeout=0.05,  #set a timeout value, None for waiting forever
                                    xonxoff=0, #enable software flow control
                                    rtscts=0,  #enable RTS/CTS flow control
                                    )
        self.serial.flushInput()
        self.serial.flushOutput()
        if verbose:
            self.logfile = open('/tmp/mrcc.log', 'a')

    ###########################################################################
    def Command(self, command, check=True):
        if verbose:
            print ("writing '%s' to MRCC... " % command),
        if True or self.baudrate == 9600:
            self.serial.write(command+'\r')
        else:
            # my impression is that above 9.6kBit/s, the MRCC ignores
            # the fifth character (well, maybe also the 10th or so,
            # but commands are usually not that long) if there is no
            # time.sleep(); e.g., sending 're 1 0 0' gives a reply 're
            # 10 0' and then 'error' as there is no module at channel
            # 10 and, of course, the 'parameter' argument is missing;
            # same for 'sc 1\r' - no reply comes in the program, but
            # if I run minicom immediately afterwards and press
            # RETURN, or if just send 'sc1\r\r' or , the scanbus
            # results are shown nicely
            cmd = command + '\r'
            l = len(cmd)
            s = 1
            for i in range(0,l,s):
                self.serial.write(cmd[i:min(i+s,l)])
                time.sleep(0.002)
            time.sleep(0.05)
        if verbose:
            print " done"

        # read lines from serial port until we get a "mrc-1>" prompt
        reply = []
        while True:
            if verbose:
                print "read from MRCC... ",
            l = self.serial.read(2)
            if verbose:
                print "got '%s' +" % l.strip('\r\n'),
            if l == "mr":
                ll = self.serial.read(4)
                if verbose:
                    print "'%s' (end)" % ll.strip('\r\n')
                reply.append(l+ll)
                break
            ll = self.serial.readline()
            if verbose:
                print "'%s'" % ll.strip('\r\n')
            x = self.serial.read(1)
            #if verbose: print "%02x at end" % ord(x)
            reply.append(l+ll)

        reply = [r.strip('\r\n').lower() for r in reply]
        if verbose:
            print command, reply
            self.logfile.write( command + " --- " + str(reply) + '\n')
            self.logfile.flush()
        if not len(reply):
            raise MRCC_Exception("no reply for command '%s'" % command)
        if check and reply[0] != command:
            raise MRCC_Exception("command '%s' not echoed ('%s')" % ( command, reply[0] ))
        return reply

    ###########################################################################
    def _CheckBusDev(self, bus, dev):
        if bus != 0 and bus != 1:
            raise MRCC_Exception("Bad bus %d (must be 0 or 1)" % bus)
        if dev < 0 or dev > 15 or dev != int(dev):
            raise MRCC_Exception("Bad dev %d (must be 0..15)" % dev)

    ###########################################################################
    def SetRC(self, bus, dev, enable_rc):
        self._CheckBusDev(bus, dev)

        cmd = "%s %d %d" % ((enable_rc and "on" or "off"), bus, dev)
        reply = self.Command(cmd)

    ###########################################################################
    def SetParameter(self, bus, dev, par, val, mirror=False):
        self._CheckBusDev(bus, dev)

        cmd = "%s %d %d %d %d" % ((mirror and "sm" or "se"), bus, dev, par, val)
        reply = self.Command(cmd)
        #return self.GetParameter(bus, dev, par, mirror)

    ###########################################################################
    def GetParameter(self, bus, dev, par, mirror=False):
        self._CheckBusDev(bus, dev)

        cmd = "%s %d %d %d" % ((mirror and "rm" or "re"), bus, dev, par)
        reply = self.Command(cmd, False) # do not check echo as reply line contains the value
        r = reply[1].split()
        if len(r) != 5 or r[0]!=cmd[0:2] or int(r[1])!=bus or int(r[2])!=dev or int(r[3])!=par:
            raise MRCC_Exception("reply %s does not reflect command '%s' '%s'" % (repr(r), cmd, cmd[0:2]))
        if verbose:
            print "=> GetParameter() yields", int(r[4])
        return int(r[4])

    ###########################################################################
    def CopyMirror(self, bus, dev):
        self._CheckBusDev(bus, dev)

        cmd = "cp %d %d" % (bus, dev)
        reply = self.Command(cmd)
        if verbose:
            print reply

    ###########################################################################
    def ScanBus(self, bus):
        if bus != 0 and bus != 1:
            raise MRCC_Exception("Bad bus %d (must be 0 or 1)" % bus)

        cmd = "sc %d" % bus
        reply = self.Command(cmd)

        # check reply
        if len(reply) != 19:
            raise MRCC_Exception("reply with != 19 lines")
        if reply[1] != ("id-scan bus %d:" % bus):
            raise MRCC_Exception("no id-scan reply")
        if reply[18] != ("mrc-1>"):
            raise MRCC_Exception("no 'mrc-1>' at end")

        # build list of modules
        modules = []
        for i in range(16):
            a, tail = reply[i+2].split(':')
            if tail != " -":
                m, r = tail.split(',')
                address = int(a)
                module_type = int(m)
                rc_state = (r != ' 0ff')
                modules.append( [address, module_type, rc_state] )
        return modules

###############################################################################
###############################################################################

class MRCC_Dummy:

    ###########################################################################
    def __init__(self, parameters):
        conf = parameters and parameters.split(':') or []
        self.mhv4_problems = not ('noMHV4problems' in conf)

        self.mhv4_bus = 1
        self.mhv4_dev = 9
        self.mhv4_rc = 1
        self.mhv4_memory = [0,0,0,0, # voltages
                            1,0,0,0, # rc on-off switch
                            20,20,20,20, # current-warning limits
                            1, # rc on/off (ignore and read from self.mhv4_rc!)
                            0, # V range (0=100V, 1=400V)
                            0,0,0,0, # polarity (0=-, 1=+)
                            -99,-99,-99,-99] # currents
        self.stm16_bus = 1
        self.stm16_dev = 0
        self.stm16_rc = 1
        self.stm16_memory = [8]*32

    ###########################################################################
    def Command(self, command, check=True):
        print "dummy: 'command' not implemented; returning '%s'" % command
        return command

    ###########################################################################
    def _IsMHV4(self, bus, dev):
        return bus==self.mhv4_bus and dev==self.mhv4_dev

    ###########################################################################
    def _IsSTM16(self, bus, dev):
        return bus==self.stm16_bus and dev==self.stm16_dev

    ###########################################################################
    def _CheckBusDev(self, bus, dev):
        if not(self._IsMHV4(bus,dev) or self._IsSTM16(bus,dev)):
            raise MRCC_Exception("Bad bus/dev %d %d" % (bus, dev))

    ###########################################################################
    def SetRC(self, bus, dev, enable_rc):
        self._CheckBusDev(bus, dev)
        if self._IsMHV4(bus,dev):
            self.mhv4_rc = enable_rc and 1 or 0
            self.mhv4_memory[44-32] = self.mhv4_rc
        elif self._IsSTM16(bus,dev):
            self.stm16_rc = enable_rc and 1 or 0

    ###########################################################################
    def SetParameter(self, bus, dev, par, val, mirror=False):
        # ignore mirror
        self._CheckBusDev(bus, dev)
        if par<0:
            raise MRCC_Exception("par %d must be >= 0 for writing" % par)
        if self._IsMHV4(bus,dev):
            if par>11:
                raise MRCC_Exception("par %d must be <= 11 for MHV4 writing" % par)
            self.mhv4_memory[par] = val
        elif self._IsSTM16(bus,dev):
            if par>31:
                raise MRCC_Exception("par %d must be <=31 for STM16 writing" % par)
            if (par%4)==2:
                raise MRCC_Exception("par %d must not fulfil par%4==2 for STM16 writing" % par)
            self.stm16_memory[par] = val

    ###########################################################################
    def GetParameter(self, bus, dev, par, mirror=False):
        # ignore mirror
        self._CheckBusDev(bus, dev)
        if self._IsMHV4(bus,dev):
            if par<32:
                raise MRCC_Exception("par %d must be >= 32 for MHV4 reading" % par)
            if par>53:
                raise MRCC_Exception("par %d must be <= 53 for MHV4 writing" % par)
            val = self.mhv4_memory[par-32]
            if not mirror and self.mhv4_problems:
                if par < 36:
                    # the voltage is usually 1% less in the real device
                    val = int(0.99*val)
                elif par >= 40 and par < 44:
                    # randomize the current warning threshold a bit
                    val = int(val + 3*(random.random()-0.5))
                if par>=50 and par<54:
                    # make up some current
                    return int(self.mhv4_memory[par-50]/10.0 + 5*(random.random()-0.5))
            return val
        elif self._IsSTM16(bus,dev):
            if par<0:
                raise MRCC_Exception("par %d must be >= 0 for STM16 reading" % par)
            if par>31:
                raise MRCC_Exception("par %d must be <=31 for STM16 writing" % par)
            if (par%4)==2:
                raise MRCC_Exception("par %d must not fulfil par%4==2 for STM16 writing" % par)
            return self.stm16_memory[par]

    ###########################################################################
    def CopyMirror(self, bus, dev):
        pass

    ###########################################################################
    def ScanBus(self, bus):
        modules = []
        if bus == self.mhv4_bus:
            modules.append( [self.mhv4_dev, 17, self.mhv4_rc] )
        if bus == self.stm16_bus:
            modules.append( [self.stm16_dev, 19, self.stm16_rc] )
        return modules

###############################################################################
def connect(device):
    if device[0:5]=='dummy':
        dummy_par = len(device)>6 and device[6:] or ""
        s = MRCC_Dummy(dummy_par)
    else:
        s = MRCC_Connection(device)
    return s

###############################################################################
if __name__ == '__main__':
    import sys
    s = connect(sys.argv[1])
    bus = 1
    if len(sys.argv)>2:
        bus = int(sys.argv[2])
    m = s.ScanBus(bus)
    print m

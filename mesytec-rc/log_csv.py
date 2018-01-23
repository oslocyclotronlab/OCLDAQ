#!/usr/bin/env python
# -*- coding: utf-8 -*-
# vim: set fileencoding=utf-8 :

import os, time, datetime

debug = False

def timestamp_values():
    ts = time.time()
    d0 = datetime.datetime(1900,1,1)
    d1 = datetime.datetime.fromtimestamp(ts)
    dt = datetime.datetime(d1.year, d1.month, d1.day)

    diff_days = (d1-d0).days + 2
    diff_day_fraction = (d1-dt).seconds/(24.0*3600)

    return ts, (diff_days + diff_day_fraction)

###############################################################################
###############################################################################

class Log_csv:
    def __init__(self):
        self.wfile  = None

    ###########################################################################
    def open(self, filename):
        self.wfile  = open( filename, "ab" )

    ###########################################################################
    def close(self):
        if self.wfile:
            self.wfile.close()
            self.wfile = None

    ###########################################################################
    def start_prepare(self):
        if debug:
            print "start_prepare"
        self.names = []
        self.items = []

    ###########################################################################
    def prepare(self, name, items):
        """Make sure that a table exists.

        items -- list of tuples (name, flag) where flag=True means integer value,
                 False= text value
        
        """

        self.names.append(name)
        self.items.append(items)

    ###########################################################################
    def stop_prepare(self):
        if debug:
            print "stop_prepare"
        self.wfile.write("\n\nTime\tDays")
        columns = []
        for idx, n in enumerate(self.names):
            for i in self.items[idx]:
                column = n + "---" + i[0]
                columns.append(column)
                self.wfile.write("\t" + column)
        self.wfile.write("\n")
        self.wfile.flush()
        os.fsync(self.wfile.fileno())
        if debug:
            print columns

    ###########################################################################
    def start_log(self):
        if debug:
            print "start_log"
        self.values = {}

    ###########################################################################
    def log(self, name, values):
        """Add values to a table.

        values -- list of tuples (name, value)
        
        """

        self.values[name] = values

    ###########################################################################
    def stop_log(self):
        if debug:
            print "stop_log"
        ts, days = timestamp_values()
        self.wfile.write(str(ts)+"\t"+str(days))
        columns = []
        for n in self.names:
            for i in self.values[n]:
                columns.append(i[1])
                self.wfile.write("\t"+ str(int(i[1])))
        self.wfile.write("\n")
        self.wfile.flush()
        os.fsync(self.wfile.fileno())
        if debug:
            print columns


###############################################################################
if __name__ == "__main__":
    debug = True

    import sys
    l = Log_csv()
    l.open(sys.argv[1])
    name = sys.argv[2]
    values = [s.split("=") for s in sys.argv[3:]]

    l.start_prepare()
    l.prepare(name, [(v[0],True) for v in values])
    l.stop_prepare()

    l.start_log()
    l.log(name, [(v[0],int(v[1])) for v in values])
    l.stop_log()

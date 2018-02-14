#!/bin/bash


# We get the root of the directory where we have the Sirius source files
TOPDIR="$(cd "$(dirname "$0")" && pwd -L)"


# We define the path to the different 'projects' within the Sirus 'package'
LIBDIR="$TOPDIR/src/lib"
MASTERDIR="$TOPDIR/src/master"
ENGINEDIR="$TOPDIR/src/engine"
XIAENGINEDIR="$TOPDIR/src/XIAengine"
RUPDATEDIR="$TOPDIR/src/rupdate"
XIASORTDIR="$TOPDIR/src/XIAonline"
SORTDIR="$TOPDIR/src/sort"
USRSORTDIR="$TOPDIR/src/user_sort"

# We are navigating to the lib director and build!
cd $LIBDIR
make

# Now for the master code OOOooOOO 'MASTER' :p
cd $MASTERDIR
make

# Next engine directory
cd $ENGINEDIR
make

# Next XIAengine directory
cd $XIAENGINEDIR
make

# Rupdate
cd $RUPDATEDIR
make

# Sort XIA directory
cd $XIASORTDIR
make
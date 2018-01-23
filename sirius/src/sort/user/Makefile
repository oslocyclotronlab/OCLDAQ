# -*- Makefile -*-
#
# compile sorting program with user_routine

# must be an absolute path
SORTDIR = $(shell pwd)/..

# sorting routine for online
#USER_ROUTINE_SRC = $(shell pwd)/user_routine_voinov.cpp
USER_ROUTINE_SRC = $(shell pwd)/user_routine_mama.cpp

# standard sorting routine for offline
#USER_OFFLINE_SRC = $(shell pwd)/user_routine_mama.cpp $(SORTDIR)/user_routine_basic.cpp
# root-based offline sorting routine
USER_OFFLINE_SRC = $(shell pwd)/user_routine_root.cpp $(shell pwd)/userroot.cpp

############################################################
# usually, no changes should be necessary below this line
############################################################

include $(SORTDIR)/Makefile.user

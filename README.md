# OCLDAQ [![DOI](https://zenodo.org/badge/DOI/10.5281/zenodo.1206106.svg)](https://doi.org/10.5281/zenodo.1206106)
The OCL DAQ software is a package of programs that are used for readout of data aquisition systems for nuclear physics experiments.
The original package contains an event builder to be ran on a dedicated Lynx computer in a VME crate. Newer verions will transition to a XIA based system.

## Pre-requisits
Currently the OCL DAQ software requires the following packages to compile:
* libCAENVME - CAEN VME library, can be downloaded from [here](http://www.caen.it/jsp/Template2/CaenProd.jsp?idmod=689&parent=43)
* SBS drivers - Go to [GE's library page](http://www.geautomation.com/library) and search for "1003". Currently there are no official support for Linux 3.xx, but the source code is avalible and one could try to compile it with 3.xx.
* X11 - Most linux distributions have X11.
* openmotif/motif/etc - Most linux distributions have openmotif installed.
* ROOT 5.xx - Can be downloaded from [here](https://root.cern). Currently the software are not compatible with ROOT 6.

# OCLDAQ [![DOI](https://zenodo.org/badge/DOI/10.5281/zenodo.6052828.svg)](https://doi.org/10.5281/zenodo.6052828)
The OCL DAQ software is a package of programs that are used for readout of data aquisition systems for nuclear physics experiments.
The original package contains an event builder to be ran on a dedicated Lynx computer in a VME crate. Newer verions will transition to a XIA based system.

## Pre-requisits
Currently the OCL DAQ software requires the following packages to compile:
* X11 - Most linux distributions have X11.
* openmotif/motif/etc - Most linux distributions have openmotif installed.
* ROOT ≥6.14 - Can be downloaded from [here](https://root.cern).
* Qt5 - Get from [qt.io](https://qt.io) or your favourite repo.

### Analog only pre-requisits (see tag 1.5.3 or older)
* libCAENVME - CAEN VME library, can be downloaded from [here](http://www.caen.it/jsp/Template2/CaenProd.jsp?idmod=689&parent=43)
* SBS drivers - Go to [GE's library page](http://www.geautomation.com/library) and search for "1003". Currently there are no official support for Linux 3.xx, but the source code is avalible and one could try to compile it with 3.xx.

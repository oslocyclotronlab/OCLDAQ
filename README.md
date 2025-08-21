# OCLDAQ [![DOI](https://zenodo.org/badge/DOI/10.5281/zenodo.6052828.svg)](https://doi.org/10.5281/zenodo.6052828)
OCLDAQ is a collection of programs required to run a full data acquisition system.
This includes user interface for run control, online analysis and spying on spectra as they are incremented.

The software targets the XIA Pixie-16 system, but parts of it can easily be modified to be used in other systems as well.

## Dependencies
* libmotif
* Qt 6
* libxt
* X11
* [ROOT](root.cern)
* The SDK from [PLX](https://github.com/xiallc/broadcom_pci_pcie_sdk)

If you are on an older distribution you may neded to install packages from the `backports` repo of your distribution.
The software has been tested and runs fine on Debian 11. In the case of Debian 11 you will need to use the `http://archive.debian.org/deb` for the `backports`repository.

Generally I've found the 8.23 version of the PLX drivers to work reliably, but feel free to try the 9.81 version as well.
If you like to use the 8.23 on a kernel newer than 5.4 you will need to make a few modifications, see this [PR](https://github.com/xiallc/broadcom_pci_pcie_sdk/pull/3). Just download the changed files from the PR and drop them in the downloaded repo.
In general, follow the how-to in this [repo](https://github.com/xiallc/broadcom_pci_pcie_sdk/blob/master/xia/doc/install.md) to install.


## Setup instructions
Before starting the setup procedure you will need to install the following packages:
 * Debian style distro:
```bash
sudo apt install libmotif-dev libxt-dev qt6-base-dev 
```

* CentOS style distro (aka. Almalinux, etc.)
```bash
sudo dnf install motif-devel qt6-qtbase-devel 
```


## Pre-requisits
Currently the OCL DAQ software requires the following packages to compile:
* X11 - Most linux distributions have X11.
* openmotif/motif/etc - Most linux distributions have openmotif installed.
* ROOT ≥6.14 - Can be downloaded from [here](https://root.cern).
* Qt5 - Get from [qt.io](https://qt.io) or your favourite repo.
# OCLDAQ

The OCL DAQ software is a suite of programs used for the readout and processing of data acquisition systems for nuclear physics experiments at the Oslo Cyclotron Laboratory.

The system has transitioned from a VME-based architecture to a XIA-based system.

## Architecture Overview

The DAQ system consists of several interconnected components:

### Core Data Path
1. **XIAengine**: The low-level interface that communicates with XIA hardware (e.g., Pixie16). It handles raw data readout and provides a shared memory interface for other components.
2. **XIAonline**: The online processing engine. It reads raw data from `XIAengine` (via shared memory or network), unpacks it, performs real-time sorting (singles and coincidences), and provides a ROOT server for live monitoring.
3. **ROOT Server**: Integrated within `XIAonline`, it allows users to visualize histograms in real-time using ROOT.

### Control & Configuration
* **master**: The central control application used to manage the acquisition process, start/stop runs, and monitor status.
* **XIAConfigurator**: A GUI tool used to configure the XIA hardware parameters (filters, baselines, etc.) via the `XIAengine`.

### Analysis & Monitoring
* **rupdate**: A utility for updating and processing ROOT files after the acquisition.
* **RateMonitor**: A Python-based tool for monitoring the event rates of the system.
* **sirius**: A supporting component for engine shared memory and network control.

## Prerequisites

### Build System
* CMake $\ge$ 3.14
* C++17 compatible compiler

### Dependencies
* **ROOT** $\ge$ 6.14 - [Download here](https://root.cern)
* **Qt5** - For GUI components (`XIAConfigurator`, `master`)
* **X11 / Motif** - Required for some legacy GUI elements
* **Other Libraries**: The project uses CPM for dependency management, automatically fetching libraries like `yaml-cpp`, `magic_enum`, and `indicators`.

## Building the Project

The project uses a top-level CMake configuration to build all components.

```bash
mkdir build && cd build
cmake ..
make -j$(nproc)
```

The binaries are typically located in the `all/` build directory.

## Component Details

For detailed information on each component, please refer to the README files in their respective directories:
* `/XIAengine`
* `/XIAonline`
* `/XIAConfigurator`
* `/master`
* `/rupdate`
* `/sirius`
* `/RateMonitor`

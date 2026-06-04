# Sirius

This subproject provides core utilities and communication infrastructure for the DAQ system, including shared memory management and network control.

## Features

- **Shared Memory Interface**: Provides mechanisms to attach to and read from the acquisition engine's shared memory (`engine_shm.h`).
- **Network Control**: A flexible TCP-based communication framework supporting both line-based (text) and binary protocols (`net_control.h`).
- **Command Execution**: Infrastructure for handling remote commands via the network.
- **Spectrum Sorting**: Utilities for sorting and processing spectra (`sort_spectra.h`).

## Structure

- `include/`: Header files defining the interfaces.
  - `engine_shm.h`: Shared memory definitions and access functions.
  - `net_control.h`: TCP server/client implementation for line and binary data.
  - `sort_spectra.h`: Spectrum sorting logic.
  - `run_command.h`: Command execution utilities.
- `src/`: Implementation of the core utilities.
  - `engine_shm.cpp`: Shared memory logic.
  - `net_control.cpp`: Network communication logic.
  - `run_command.cpp`: Command execution logic.
  - `sort_spectra.cpp`: Spectrum sorting implementation.

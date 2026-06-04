# Master Control

This subproject provides the master control logic for the DAQ system, managing the acquisition engine and its lifecycle.

## Features

- Acquisition control: Start, stop, reload, and status monitoring.
- Output management: Configure output filenames and directories.
- Connectivity: Manage connections to the acquisition engine.
- Storage and logging: Handle data storage and system logging.

## Structure

- `src/`: Contains the implementation of the master control logic.
  - `m_engine.cpp/h`: Core engine management.
  - `acq_*.cpp`: Various acquisition control functions (start, stop, clear, dump, etc.).
  - `net_client.cpp/h`: Network client for communication.
  - `io_xtapp.cpp/h`: I/O interface for the application.

# XIA Online

This subproject is the online data processing and sorting application for the XIA DAQ system. It reads raw data from the acquisition engine and processes it through a pipeline of tasks.

## Features

- **Data Pipeline**: Implements a multi-threaded processing pipeline using a thread pool:
  - `Unpacker`: Extracts raw data into events.
  - `Buffer`: Temporarily stores unpacked data.
  - `Splitter`: Splits data into time-based chunks.
  - `SortSingles`: Processes single-hit spectra.
  - `Trigger`: Identifies trigger events.
  - `SortCoincidence`: Processes coincidence spectra.
- **Real-time Monitoring**: Provides a visual representation of buffer occupancy using progress bars.
- **ROOT Integration**: Includes a `ROOTServer` to provide real-time access to processed histograms.
- **Remote Control**: Implements a command server for remote operations (e.g., `clear`, `dump`, `change_cwd`).
- **Flexible Input**: Supports both shared memory (local) and TCP (network) data sources.

## Structure

- `include/XIAReader/`: Core processing logic.
  - `Configuration/`: Configuration management.
  - `Format/`: Data format definitions.
  - `Tasks/`: Implementation of the pipeline tasks.
- `src/`: Application source code.
  - `main.cpp`: Entry point and pipeline orchestration.
  - `ROOTServer.cpp/h`: ROOT server implementation.
  - `XIAReader/`: Implementation of the reader components.
- `test_reader/`: Tools for testing the reader logic.

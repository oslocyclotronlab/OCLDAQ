# Rate Monitor

This subproject is a Python-based daemon that monitors input and output rates from XIA modules and pushes them to an InfluxDB database.

## Features

- **Real-time Monitoring**: Uses `watchdog` to monitor the filesystem for changes to `rates.csv` files.
- **InfluxDB Integration**: Automatically parses CSV data and writes it to an InfluxDB bucket (`oscar`).
- **Systemd Support**: Can be run as a systemd daemon.
- **Custom Tagging**: Maps module and channel IDs to human-readable names (e.g., `labr`, `deltaE`, `guardRing`).

## Installation

This project uses a `setup.py` file for installation.

```bash
pip install .
```

## Usage

Run the monitor with the following arguments:

```bash
RateMonitor --host <influxdb_host> --token <influxdb_token> [-v] [-d]
```

- `--host`: Hostname of the InfluxDB HTTP API (default: `https://rates.ocl.wtf`).
- `--token`: InfluxDB access token.
- `-v`: Verbose output.
- `-d`: Run as a systemd daemon.

## Structure

- `RateMonitor/`: Core Python package.
  - `InfluxDBRateMonitor.py`: Main logic for monitoring and database writing.
  - `arg_parser.py`, `event_handler.py`: Helper modules.
- `tests/`: Test data (e.g., `scalers.csv`).
- `RateMonitor.service`: Systemd service unit file.

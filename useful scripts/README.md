# Useful Scripts

This directory contains standalone Python scripts for monitoring and managing the DAQ system's output.

## Contents

- `file_monitor.py`: Monitors the growth of `sirius-*.data` files in real-time, reporting file sizes, growth rates, and overall disk usage. It also attempts to read the beam rate from `scalers.csv`.
- `rate_monitor.py`: A standalone script for monitoring rates (likely a precursor or alternative to the `RateMonitor` subproject).

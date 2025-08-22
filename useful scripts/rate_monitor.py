#!/usr/bin/env python3

import argparse
import yaml
import csv
import time
from collections import deque
from datetime import datetime, timedelta
from watchdog.observers import Observer
from watchdog.events import FileSystemEventHandler
import os
import sys


class CSVHandler(FileSystemEventHandler):
    """Watchdog event handler for CSV file changes"""

    def __init__(self, csv_file, detector_map, history, history_minutes, use_column):
        super().__init__()
        self.csv_file = csv_file
        self.detector_map = detector_map
        self.history = history
        self.history_minutes = history_minutes
        self.use_column = use_column

    def on_modified(self, event):
        if os.path.abspath(event.src_path) == os.path.abspath(self.csv_file):
            try:
                self.process_csv()
            except Exception as e:
                print(f"Error processing CSV: {e}", file=sys.stderr)

    def process_csv(self):
        # Get timestamp from file modification time
        ts = datetime.fromtimestamp(os.path.getmtime(self.csv_file))

        # Read CSV with standard library
        rates = {}
        with open(self.csv_file, newline="") as f:
            reader = csv.DictReader(f)
            for row in reader:
                mod = int(row["module"])
                ch = int(row["channel"])
                inp = float(row["input"])
                outp = float(row["output"])
                rate = inp if self.use_column == "input" else outp
                rates[(mod, ch)] = rate

        # Update history
        for (mod, ch), det_id in self.detector_map.items():
            if (mod, ch) in rates:
                self.history[det_id].append((ts, rates[(mod, ch)]))

        # Prune old entries
        cutoff = ts - timedelta(minutes=self.history_minutes)
        for det_id in self.history:
            while self.history[det_id] and self.history[det_id][0][0] < cutoff:
                self.history[det_id].popleft()

        # Display
        self.display(rates, ts)

    def compute_avg(self, det_id):
        if not self.history[det_id]:
            return 0.0
        return sum(v for _, v in self.history[det_id]) / len(self.history[det_id])

    def display(self, rates, ts):
        print("\n" + "=" * 60)
        print(f"Updated: {ts.strftime('%Y-%m-%d %H:%M:%S')}")
        print("Detector | Current | Avg (last {} min)".format(self.history_minutes))
        print("-" * 60)
        for (mod, ch), det_id in sorted(self.detector_map.items(), key=lambda x: x[1]):
            current = rates.get((mod, ch), 0.0)
            avg = self.compute_avg(det_id)
            print(f"{det_id:8d} | {current:7.1f} | {avg:12.1f}")


def build_detector_map(setup):
    """Build (module,channel) → detectorID mapping"""
    detector_map = {}
    for crate in setup["setup"]["crates"]:
        slot = crate["slot"]
        module = slot - 2  # mapping: slot 2 → module 0, slot 3 → module 1, etc.
        for det in crate["detectors"]:
            ch = det["channel"]
            det_id = det["detectorID"]
            detector_map[(module, ch)] = det_id
    return detector_map


def main():
    parser = argparse.ArgumentParser(description="DAQ Rate Monitor")
    parser.add_argument("--setup", default="setup.yml", required=True, help="Path to setup.yaml file")
    parser.add_argument("--rate", default="scalers.csv", required=True, help="Path to DAQ CSV file")
    parser.add_argument("--history", type=int, default=2, help="History length in minutes")
    parser.add_argument("--column", choices=["input", "output"], default="input",
                        help="Which column to use for count rate (input/output)")
    args = parser.parse_args()

    # Load YAML
    with open(args.setup, "r") as f:
        setup = yaml.safe_load(f)

    detector_map = build_detector_map(setup)
    history = {det_id: deque() for det_id in detector_map.values()}

    # Setup watchdog
    event_handler = CSVHandler(args.rate, detector_map, history, args.history, args.column)
    observer = Observer()
    observer.schedule(event_handler, path=os.path.dirname(os.path.abspath(args.csv)) or ".", recursive=False)
    observer.start()

    print(f"Monitoring {args.csv} (column: {args.column}) with {args.history} min history...")
    print("Press Ctrl+C to stop.")

    try:
        while True:
            time.sleep(1)
    except KeyboardInterrupt:
        observer.stop()
    observer.join()


if __name__ == "__main__":
    main()

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
from tabulate import tabulate


class CSVHandler(FileSystemEventHandler):
    """Watchdog event handler for CSV file changes"""

    def __init__(self, csv_file, detector_map, history, history_minutes):
        super().__init__()
        self.csv_file = csv_file
        self.detector_map = detector_map
        self.history = history
        self.history_minutes = history_minutes

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
        rates_in = {}
        rates_out = {}
        with open(self.csv_file, newline="") as f:
            reader = csv.DictReader(f)
            for row in reader:
                mod = int(row["module"])
                ch = int(row["channel"])
                inp = float(row["input"])
                outp = float(row["output"])
                rates_in[(mod, ch)] = inp
                rates_out[(mod, ch)] = outp

        # Update history
        for (mod, ch), (det_type, det_id) in self.detector_map.items():
            if (mod, ch) in rates_in:
                self.history[det_id]["input"].append((ts, rates_in[(mod, ch)]))
            if (mod, ch) in rates_out:
                self.history[det_id]["output"].append((ts, rates_out[(mod, ch)]))

        # Prune old entries
        cutoff = ts - timedelta(minutes=self.history_minutes)
        for det_id in self.history:
            for col in ("input", "output"):
                while self.history[det_id][col] and self.history[det_id][col][0][0] < cutoff:
                    self.history[det_id][col].popleft()

        # Display
        self.display(rates_in, rates_out, ts)

    def compute_avg(self, det_id, col):
        if not self.history[det_id][col]:
            return 0.0
        return sum(v for _, v in self.history[det_id][col]) / len(self.history[det_id][col])

    def display(self, rates_in, rates_out, ts):
        # Clear screen
        sys.stdout.write("\033[2J\033[H")
        sys.stdout.flush()

        table = []
        headers = ["Detector", "Type", "Input", f"Avg Input ({self.history_minutes}m)",
                   "Output", f"Avg Output ({self.history_minutes}m)"]

        for (mod, ch), (det_type, det_id) in sorted(self.detector_map.items(), key=lambda x: x[1]):
            cur_in = rates_in.get((mod, ch), 0.0)
            avg_in = self.compute_avg(det_id, "input")
            cur_out = rates_out.get((mod, ch), 0.0)
            avg_out = self.compute_avg(det_id, "output")
            table.append([det_id, det_type, f"{cur_in:.1f}", f"{avg_in:.1f}",
                          f"{cur_out:.1f}", f"{avg_out:.1f}"])

        print(f"Updated: {ts.strftime('%Y-%m-%d %H:%M:%S')}")
        print(tabulate(table, headers=headers, tablefmt="fancy_outline", stralign="right", numalign="right"))


def build_detector_map(setup):
    """Build (module,channel) → (det_type, detectorID) mapping"""
    detector_map = {}
    for crate in setup["setup"]["crates"]:
        for slot in crate["slots"]:
            slotID = slot["slot"]
            module = slotID - 2  # mapping: slot 2 → module 0, slot 3 → module 1, etc.
            for det in slot["detectors"]:
                ch = det["channel"]
                det_id = det["detectorID"]
                det_type = det["type"]
                detector_map[(module, ch)] = (det_type, det_id)
    return detector_map


def main():
    parser = argparse.ArgumentParser(description="DAQ Rate Monitor")
    parser.add_argument("--setup", default="setup.yml", help="Path to setup.yaml file")
    parser.add_argument("--rate", default="scalers.csv", help="Path to DAQ CSV file")
    parser.add_argument("--history", type=int, default=2, help="History length in minutes")
    args = parser.parse_args()

    # Load YAML
    with open(args.setup, "r") as f:
        setup = yaml.safe_load(f)

    detector_map = build_detector_map(setup)

    # History holds separate deques for input/output per detector
    history = {det_id: {"input": deque(), "output": deque()} for _, det_id in detector_map.values()}

    # Setup watchdog
    event_handler = CSVHandler(args.rate, detector_map, history, args.history)
    observer = Observer()
    observer.schedule(event_handler, path=os.path.dirname(os.path.abspath(args.rate)) or ".", recursive=False)
    observer.start()

    print(f"Monitoring {args.rate} with {args.history} min history...")
    print("Press Ctrl+C to stop.")

    try:
        while True:
            time.sleep(1)
    except KeyboardInterrupt:
        observer.stop()
    observer.join()


if __name__ == "__main__":
    main()

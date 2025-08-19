#!/usr/bin/env python3
import os
import time
import glob
import re
import shutil
import csv
from datetime import datetime
from collections import defaultdict


# ANSI color codes
RED = "\033[91m"
GREEN = "\033[92m"
RESET = "\033[0m"

def sort_key(key):
    # Extract timestamp and optional big-x
    m = re.match(r"sirius-(\d{8}-\d{6})(?:-big-(\d+))?\.data", key[0])
    if not m:
        return (datetime.min, 1, float("inf"))  # fallback
    
    ts_str, big_idx = m.groups()
    ts = datetime.strptime(ts_str, "%Y%m%d-%H%M%S")
    
    if big_idx is None:
        return (ts, 0, 0)  # plain .data first
    else:
        return (ts, 1, int(big_idx))  # then big-x by index

def human_readable_size(num_bytes):
    """Convert bytes into human-readable string."""
    for unit in ['B', 'KB', 'MB', 'GB', 'TB']:
        if num_bytes < 1024.0:
            return f"{num_bytes:3.1f}{unit}"
        num_bytes /= 1024.0
    return f"{num_bytes:.1f}PB"

def render_usage_bar(used, free, bar_length=40):
    """Render a monochrome usage bar with percentages."""
    total = used + free
    if total == 0:
        return " " * bar_length + " 0% used / 0% free"

    used_pct = used / total * 100
    free_pct = 100 - used_pct

    used_len = int(bar_length * used / total)
    free_len = bar_length - used_len

    bar = f"{'█'*used_len}{'░'*free_len}"
    return f"{bar}  {used_pct:.1f}% used / {free_pct:.1f}% free"

def read_beam_rate(csv_filename="scalers.csv"):
    with open(csv_filename) as csvfile:
        ratesreader = csv.reader(csvfile, delimiter=",")
        for row in ratesreader:
            if row[0] == '2' and row[1] == '15':
                return int(row[2])

def find_latest_timestamp(folder):
    """Find the most recent yyyymmdd-HHMMSS timestamp from sirius-*.data files."""
    files = glob.glob(os.path.join(folder, "sirius-*.data")) + \
            glob.glob(os.path.join(folder, "sirius-*-big-*.data"))

    timestamps = []
    for f in files:
        m = re.search(r"sirius-(\d{8}-\d{6})", os.path.basename(f))
        if m:
            timestamps.append(m.group(1))

    return max(timestamps) if timestamps else None

def scan_files(folder, ts_str):
    """Find matching files and return {filename: size}."""
    pattern1 = os.path.join(folder, f"sirius-{ts_str}.data")
    pattern2 = os.path.join(folder, f"sirius-{ts_str}-big-*.data")
    files = glob.glob(pattern1) + glob.glob(pattern2)
    sizes = {}
    for f in files:
        try:
            sizes[f] = os.path.getsize(f)
        except FileNotFoundError:
            continue
    return sizes

def monitor(folder, interval=1, current_range=2):
    prev_sizes = defaultdict(int)
    prev_time = time.time()
    current_ts = None

    while True:
        ts_str = find_latest_timestamp(folder)
        if not ts_str:
            os.system("clear" if os.name == "posix" else "cls")
            print(f"No sirius-*.data files found in {folder}")
            time.sleep(interval)
            continue

        if ts_str != current_ts:
            # reset tracking if a new timestamp set is detected
            prev_sizes.clear()
            current_ts = ts_str

        sizes = scan_files(folder, ts_str)
        now = time.time()
        elapsed = now - prev_time

        # disk usage stats
        du = shutil.disk_usage(folder)
        free_space = du.free
        total_disk = du.total
        used_space = du.used

        os.system("clear" if os.name == "posix" else "cls")
        print(f"Monitoring folder: {folder} (latest timestamp: {ts_str})")
        print("=" * 75)

        total_size = 0
        total_growth = 0

        if not sizes:
            print("No matching files found for this timestamp.")
        else:
            for f, size in sorted(sizes.items(), key=sort_key):
                prev_size = prev_sizes[f]
                growth = (size - prev_size) / elapsed if elapsed > 0 else 0
                total_size += size
                total_growth += max(0, growth)
                print(f"{os.path.basename(f):45s} {human_readable_size(size):>10} (+{human_readable_size(growth)}/s)")
                prev_sizes[f] = size

            print("-" * 75)
            print(f"{'TOTAL (files)':45s} {human_readable_size(total_size):>10} (+{human_readable_size(total_growth)}/s)")
            print()
            print(f"{'Disk used (all filesystems)':45s} {human_readable_size(du.used):>10}")
            print(f"{'Disk free':45s} {human_readable_size(free_space):>10}")
            print(f"{'Disk total':45s} {human_readable_size(total_disk):>10}")
            print(f"{'Disk usage':45s} {render_usage_bar(used_space, free_space)}")

            try:
                print(f"{'Beam rate':45s} {read_beam_rate()*current_range/1000.} uA")
            except:
                pass

        prev_time = now
        time.sleep(interval)

if __name__ == "__main__":
    import argparse
    parser = argparse.ArgumentParser(description="Monitor sirius data files (latest timestamp) and report size + growth rate + disk usage.")
    parser.add_argument("folder", help="Folder to monitor")
    parser.add_argument("--interval", type=int, default=1, help="Update interval in seconds (default: 1)")
    parser.add_argument("--current_range", type=float, default=2, help="Current integrator range in uA")
    args = parser.parse_args()

    monitor(args.folder, args.interval, args.current_range)

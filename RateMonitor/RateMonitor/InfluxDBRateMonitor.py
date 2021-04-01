import sys
import time
import copy
import logging
import argparse
import pandas as pd
from pathlib import Path
from datetime import datetime
from pytz import UTC
from influxdb_client import InfluxDBClient, WriteOptions
from influxdb_client.client.write_api import ASYNCHRONOUS, SYNCHRONOUS
from watchdog.observers import Observer
from watchdog.events import LoggingEventHandler, FileSystemEventHandler

formatter = logging.Formatter('%(asctime)s - %(name)s - %(funcName)s - %(levelname)s - %(message)s')  # noqa
output_handler = logging.StreamHandler()
systemd_handler = None
try:
    import systemd
    systemd_handler = systemd.journal.JournalHandler()
except ModuleNotFoundError:
    pass

output_handler.setFormatter(formatter)


def GetTags():
    mapping = {}
    for i in range(15):
        mapping[(0, i)] = {"name": f"labr{i:02d}", "type": "labr"}
        mapping[(1, i)] = {"name": f"labr{i+15:02d}", "type": "labr"}

    mapping[(0, 15)] = {"name": "unused0", "type": "unused"}
    mapping[(1, 15)] = {"name": "unused0", "type": "unused"}

    for i in range(8):
        for j in range(8):
            mapping[(2+int(i/2), j)] = {"name": f"deltaE_{i}_{j}",
                                        "type": "deltaE"}
    for i in range(8):
        mapping[(6, i)] = {"name": f"guardRing{i}", "type": "guardRing"}
        mapping[(6, i+1)] = {"name": f"eDet{i+1}", "type": "eDet"}
    for i in range(4):
        mapping[(6, i)] = {"name": "PPAC{i}", "type": "PPAC"}
    return mapping


def WriteTag(tags):
    outstr = ""
    for key, value in tags.items():
        outstr += f",{key}={value}"
    return outstr


class EventHandler(FileSystemEventHandler):

    def __init__(self, host, token, log_handler,
                 log_level=logging.INFO):
        self.logger = logging.getLogger("RateMonitor")
        self.logger.propagate = False
        self.logger.addHandler(log_handler)
        self.logger.setLevel(log_level)
        self.logger.debug(f"Trying to connect to '{host}' using token '{token}'")
        self._client = InfluxDBClient(url=host, token=token, org="OCL")

        health = self._client.health()
        if health.status == 'pass':
            self.logger.info(f"Connected to InfluxDB version {health.version} at '{host}'")  # noqa
        else:
            self.logger.error(f"Failed to connect to InfluxDB at '{host}'")

        # Get the writer API
        self._writer = self._client.write_api(write_options=SYNCHRONOUS)

        # Done configuring the InfluxDB client
        self.tags = GetTags()

    def csv_to_line(self, fname, exp_name):
        df = pd.read_csv(fname)
        rates = []
        now = datetime.now(UTC)
        for _, row in df.iterrows():
            module = int(row['module'])
            channel = int(row['channel'])
            tags = copy.deepcopy(self.tags[(module, channel)])
            tags['experiment'] = exp_name
            rates.append({
                "measurement": "rate",
                "tags": tags,
                "fields": {"input": row['input'],
                           "output": row['output']},
                "time": now
                })
        return rates

    def on_any_event(self, event):
        self.logger.debug(event)

        # We do not look at directories
        if event.is_directory:
            return

        # If event type isn't created
        # or modified we will return
        if not (event.event_type == "created" or event.event_type == "modified"):  # noqa
            return

        # Next we will get the full path
        # to the file and check that the
        # file name is equal to rates.csv
        path = Path(event.src_path)
        self.logger.debug(f"File name: {path.name}")
        if path.name != "rates.csv":
            return

        # Now we can actually do something fun!
        entries = []
        try:
            entries += self.csv_to_line(path, path.parent.name)
        except Exception as e:
            self.logger.error(f"Error parsing {path}: '{str(e)}'")

        self.logger.debug(f"Read from file:\n{entries}")

        try:
            res = self._writer.write(bucket="oscar", record=entries)
            self.logger.debug(f"Write to DB resulted in: {res}")
        except Exception as e:
            self.logger.error(f"Could not write to database: '{str(e)}'")

        return

    def watch(self, src):
        observer = Observer()
        observer.schedule(self, src, recursive=True)
        observer.start()
        try:
            while True:
                time.sleep(1)
        except Exception:
            observer.stop()
        observer.join()


def my_watch(src):
    event_handler = EventHandler()
    observer = Observer()
    observer.schedule(event_handler, src, recursive=True)
    observer.start()
    try:
        while True:
            time.sleep(1)
    except Exception:
        observer.stop()
    observer.join()


def parse_args():
    """ Parse args to be used by "main"
    """

    parser = argparse.ArgumentParser(
        description="A deamon to monitor input/output rates from XIA modules")

    parser.add_argument("--host", type=str, required=False,
                        default="https://rates.ocl.wtf",
                        help="Hostname of the InfluxDB HTTP API")
    parser.add_argument("--token", type=str, required=False,
                        default="nh5Za_nA1jQ4y5Qj1tcNhV8UIb07iJ2SxDTP7y0sq4nTg3S9povxLrRymT2taK7Y8R4Z3cuMI_Y1ubZN4PLd1w==",  # noqa
                        help="InfluxDB access token")
    parser.add_argument("-v", action='store_true', help="Verbose output")
    parser.add_argument("-d", action='store_true',
                        help="Run as systemd daemon")
    return parser.parse_args()


if __name__ == "__main__":

    args = parse_args()
    log_level = logging.INFO
    handler = output_handler
    if args.v:
        log_level = logging.DEBUG
    if args.d:
        handler = systemd_handler

    evt_handler = EventHandler(host=args.host, token=args.token,
                               log_handler=handler, log_level=log_level)

    evt_handler.watch("./")

    """if len(sys.argv) == 2:
        my_watch(sys.argv[1])
    path = sys.argv[1] if len(sys.argv) > 1 else '.'
    event_handler = EventHandler()
    observer = Observer()
    observer.schedule(event_handler, path, recursive=True)
    observer.start()
    try:
        while True:
            time.sleep(1)
    except KeyboardInterrupt:
        observer.stop()
    observer.join()"""
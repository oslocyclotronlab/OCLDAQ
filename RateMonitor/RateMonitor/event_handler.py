import time
import copy
import logging
import pandas as pd
from pathlib import Path
from datetime import datetime
from pytz import UTC
from influxdb_client import InfluxDBClient, WriteOptions
from influxdb_client.client.write_api import ASYNCHRONOUS, SYNCHRONOUS
from watchdog.observers import Observer
from watchdog.events import LoggingEventHandler, FileSystemEventHandler


def GetTags():
    mapping = {}
    for i in range(15):
        mapping[(0, i)] = {"name": f"labr{i:02d}", "type": "labr"}
        mapping[(1, i)] = {"name": f"labr{i+15:02d}", "type": "labr"}

    mapping[(0, 15)] = {"name": "unused0", "type": "unused"}
    mapping[(1, 15)] = {"name": "unused0", "type": "unused"}

    for i in range(4):
        for j in range(8):
            mapping[(2+i, j)] = {"name": f"deltaE_{i}_{j}",
                                 "type": "deltaE"}
            mapping[(2+i, j+8)] = {"name": f"deltaE_{2*i}_{j}",
                                   "type": "deltaE"}
    for i in range(8):
        mapping[(6, 2*i)] = {"name": f"guardRing{i}", "type": "guardRing"}
        mapping[(6, 2*i+1)] = {"name": f"eDet{i+1}", "type": "eDet"}
    for i in range(4):
        mapping[(6, i)] = {"name": "PPAC{i}", "type": "PPAC"}
    return mapping


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
        if not (event.event_type == "created" or event.event_type == "modified"
                or event.event_type == "closed"):  # noqa
            return

        # Next we will get the full path
        # to the file and check that the
        # file name is equal to rates.csv
        path = Path(event.src_path)
        self.logger.debug(f"File name: {path.name}")
        if path.name != "scalers.csv":
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
        self.logger.info(f"Monitoring started in folder '{src}'")
        observer = Observer()
        observer.schedule(self, src, recursive=True)
        observer.start()
        try:
            while True:
                time.sleep(1)
        except Exception:
            observer.stop()
        observer.join()

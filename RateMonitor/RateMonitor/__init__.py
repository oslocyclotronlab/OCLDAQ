import logging
import sys


from .arg_parser import ArgParser
from .event_handler import EventHandler


formatter = logging.Formatter('%(asctime)s - %(name)s - %(funcName)s - %(levelname)s - %(message)s')  # noqa
output_handler = logging.StreamHandler()
systemd_handler = None
try:
    import cysystemd as systemd
    systemd_handler = systemd.journal.JournaldLogHandler()
    systemd_handler.setFormatter(formatter)
except Exception:
    pass


output_handler.setFormatter(formatter)


def main():

    args = ArgParser()

    log_level = logging.INFO
    if args.v:
        log_level = logging.DEBUG

    handler = output_handler
    if args.d:
        if systemd_handler is None:
            print("Could not find systemd try again, but not as daemon")
            sys.exit(1)
        handler = systemd_handler

    event_handler = EventHandler(host=args.host, token=args.token,
                                 log_handler=handler, log_level=log_level)

    if args.d:
        from cysystemd.daemon import notify, Notification
        notify(Notification.READY)
    event_handler.watch(args.src)

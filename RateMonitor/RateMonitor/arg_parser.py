import argparse


def ArgParser():
    """ Parse args to be used by "main"
    """

    parser = argparse.ArgumentParser(
        description="A deamon to monitor input/output rates from XIA modules")

    parser.add_argument("--host", type=str, required=False,
                        default="https://rates.ocl.wtf",
                        help="Hostname of the InfluxDB HTTP API")
    parser.add_argument("--token", type=str, required=True,  # noqa
                        help="InfluxDB access token")
    parser.add_argument("-v", action='store_true', help="Verbose output")
    parser.add_argument("-d", action='store_true',
                        help="Run as systemd daemon")
    parser.add_argument("--src", type=str, required=False,
                        default="/mnt/current", help="Mount point to monitor")
    return parser.parse_args()

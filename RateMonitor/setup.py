from setuptools import find_packages, setup, Command

NAME = "RateMonitor"
DESCRIPTION = "A simple daemon for monitoring count rates at OCL"
URL = "https://github.com/oslocyclotronlab/OCLDAQ/RateMonitor"
EMAIL = "vetlewi@fys.uio.no"
AUTHOR = "Vetle Wegner Ingeberg"
VERSION = None

REQUIRED = [
    "pandas",
    "pytz",
    "influxdb_client",
    "watchdog",
    "cysystemd;platform_system=='Linux'"]


about = {}
if not VERSION:
    with open('./'+NAME+'/__version__.py') as f:
        exec(f.read(), about)
else:
    about['__version__'] = VERSION

setup(name=NAME, version=about['__version__'],
      description=DESCRIPTION,
      author=AUTHOR, author_email=EMAIL,
      packages=["RateMonitor"],
      url=URL, install_requires=REQUIRED,
      include_package_data=True,
      scripts=['bin/RateMonitor'])
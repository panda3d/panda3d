#!/usr/bin/env python
"""
Tests a .whl file by installing it and pytest into a virtual environment and
running the test suite.

Requires pip to be installed, as well as 'virtualenv' on Python 2.
"""

import os
import sys
import shutil
import subprocess
import tempfile
from optparse import OptionParser


def test_wheel(wheel, verbose=False):
    envdir = tempfile.mkdtemp(prefix="venv-")
    print("Setting up virtual environment in {0}".format(envdir))
    sys.stdout.flush()

    # Create a virtualenv.
    if sys.version_info >= (3, 0):
        subprocess.call([sys.executable, "-B", "-m", "venv", "--clear", envdir])
    else:
        subprocess.call([sys.executable, "-B", "-m", "virtualenv", "--clear", envdir])

    # Determine the path to the Python interpreter.
    if sys.platform == "win32":
        python = os.path.join(envdir, "Scripts", "python.exe")
    else:
        python = os.path.join(envdir, "bin", "python")

    # Upgrade pip inside the environment too.
    if subprocess.call([python, "-m", "pip", "install", "-U", "pip"]) != 0:
        shutil.rmtree(envdir)
        sys.exit(1)

    # Install pytest into the environment, as well as our wheel.
    if subprocess.call([python, "-m", "pip", "install", "pytest", wheel]) != 0:
        shutil.rmtree(envdir)
        sys.exit(1)

    # Run the test suite.
    test_cmd = [python, "-m", "pytest", "tests"]
    if verbose:
        test_cmd.append("--verbose")

    exit_code = subprocess.call(test_cmd)
    shutil.rmtree(envdir)

    if exit_code != 0:
        sys.exit(exit_code)


if __name__ == "__main__":
    parser = OptionParser(usage="%prog [options] file...")
    parser.add_option('', '--verbose', dest = 'verbose', help = 'Enable verbose output', action = 'store_true', default = False)
    (options, args) = parser.parse_args()

    if not args:
        parser.print_usage()
        sys.exit(1)

    for arg in args:
        test_wheel(arg, verbose=options.verbose)

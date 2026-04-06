import os
import subprocess
import sys


def _get_executable(name):
    return os.path.join(os.path.dirname(__file__), name)

def _run(name):
    executable = _get_executable(name)
    return subprocess.call([executable] + sys.argv[1:])

def main():
    raise SystemExit(_run("interrogate"))

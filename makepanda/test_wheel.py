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
    pip_pkg = "pip"
    if sys.version_info[0:2] == (3, 4):
        pip_pkg = "pip<19.2"

    if subprocess.call([python, "-m", "pip", "install", "-U", pip_pkg]) != 0:
        shutil.rmtree(envdir)
        sys.exit(1)

    # Temp hack to patch issue pypa/pip#6885 in pip 19.2.2 and Python 3.8.
    if sys.platform == "win32" and "-cp38-cp38-" in wheel and os.path.isdir(os.path.join(envdir, "Lib", "site-packages", "pip-19.2.2.dist-info")):
        pep425tags = os.path.join(envdir, "Lib", "site-packages", "pip", "_internal", "pep425tags.py")
        if os.path.isfile(pep425tags):
            data = open(pep425tags, "r").read()
            data = data.replace(" m = 'm'\n", " m = ''\n")
            open(pep425tags, "w").write(data)

    # Install pytest into the environment, as well as our wheel.
    packages = [wheel]
    if sys.version_info >= (3, 10):
        packages += ["pytest>=6.2.4"]
    else:
        packages += ["pytest"]

    if sys.version_info[0:2] == (3, 4):
        if sys.platform == "win32":
            packages += ["colorama==0.4.1"]

        # See https://github.com/python-attrs/attrs/pull/807
        packages += ["attrs<21"]

    if subprocess.call([python, "-m", "pip", "install"] + packages) != 0:
        shutil.rmtree(envdir)
        sys.exit(1)

    # Run the test suite.
    test_cmd = [python, "-m", "pytest", "tests"]
    if verbose:
        test_cmd.append("--verbose")

    # Put the location of the python DLL on the path, for deploy-stub test
    # This is needed because venv does not install a copy of the python DLL
    env = None
    if sys.platform == "win32":
        deploy_libs = os.path.join(envdir, "Lib", "site-packages", "deploy_libs")
        if os.path.isdir(deploy_libs):
            # We have to do this dance because os.environ is case insensitive
            env = dict(os.environ)
            for key, value in env.items():
                if key.upper() == "PATH":
                    env[key] = deploy_libs + ";" + value
                    break
            else:
                env["PATH"] = deploy_libs

    exit_code = subprocess.call(test_cmd, env=env)
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
